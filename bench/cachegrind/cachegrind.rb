#!/usr/bin/env ruby
# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
# Runs benchmarks with cachegrind and compares the instruction counts with a baseline.
# See README.md in this directory.
#
# The benchmarks must be built with DYNAMIX_BENCH_CACHEGRIND, which makes them enable
# instrumentation only for the measured part. Each benchmark is run separately with
# a single sample and a single dimension, so the instruction count of a run is the count of
# the measured part of a single benchmark.
require 'etc'
require 'json'
require 'open3'
require 'optparse'
require 'tmpdir'

# returns an array of [suite, benchmark] pairs
def list_benchmarks(exe)
  out, status = Open3.capture2(exe, '--list')
  raise "#{exe} --list failed" unless status.success?
  ret = []
  suite = nil
  out.each_line do |line|
    if line.start_with?('    ')
      ret << [suite, line.strip]
    elsif line.start_with?('  ') && line.rstrip.end_with?(':')
      suite = line.strip[0...-1]
    end
  end
  ret
end

# returns the instruction count of the measured part of a benchmark
def run_benchmark(valgrind, exe, suite, bench, iters, out_file)
  cmd = [
    valgrind, '--tool=cachegrind', '--cache-sim=no', '--instr-at-start=no',
    "--cachegrind-out-file=#{out_file}",
    exe, "--run-suite=#{suite}", "--run-only=#{bench}", "--iters=#{iters}", '--samples=1',
  ]
  out, err, status = Open3.capture3(*cmd)
  raise "#{exe} #{suite}/#{bench} failed with #{status.exitstatus}:\n#{out}\n#{err}" unless status.success?
  File.foreach(out_file) do |line|
    m = line.match(/^summary:\s+(\d+)/)
    return m[1].to_i if m
  end
  raise "no summary in cachegrind output for #{exe} #{suite}/#{bench}"
end

# 1234.56 -> "1,234.6"
def fmt_count(n)
  int, frac = format('%.1f', n).split('.')
  int = int.reverse.scan(/\d{1,3}/).join(',').reverse
  "#{int}.#{frac}"
end

def fmt_percent(x, decimals, sign: false)
  format("%#{sign ? '+' : ''}.#{decimals}f%%", x * 100)
end

options = {
  valgrind: 'valgrind',
  baseline: nil,
  output: nil,
  update: false,
  tolerance: 0.02,
  iters: 1024,
  jobs: Etc.nprocessors,
}

parser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$PROGRAM_NAME} [options] exe [exe...]\nRun DynaMix benchmarks with cachegrind"
  opts.on('--valgrind PATH', 'valgrind executable') { |v| options[:valgrind] = v }
  opts.on('--baseline FILE', 'baseline json file (required)') { |v| options[:baseline] = v }
  opts.on('--output FILE', 'write the results to this json file') { |v| options[:output] = v }
  opts.on('--update', 'overwrite the baseline with the results') { options[:update] = true }
  opts.on('--tolerance X', Float,
          'relative change of the instruction count which is considered a regression (default 0.02)') do |v|
    options[:tolerance] = v
  end
  opts.on('--iters N', Integer, 'iterations (dimension) for each benchmark (default 1024)') { |v| options[:iters] = v }
  opts.on('--jobs N', Integer, 'number of parallel valgrind runs (default: number of cores)') { |v| options[:jobs] = v }
end
exes = parser.parse(ARGV)

if !options[:baseline] || exes.empty?
  puts parser
  exit 1
end

iters = options[:iters]
tolerance = options[:tolerance]

jobs = exes.flat_map do |exe|
  name = File.basename(exe, '.*')
  list_benchmarks(exe).map { |suite, bench| { exe: exe, key: "#{name}/#{suite}/#{bench}", suite: suite, bench: bench } }
end

puts "Running #{jobs.size} benchmarks with cachegrind..."
$stdout.flush

# run the benchmarks in parallel with a pool of threads
results = {}
Dir.mktmpdir do |tmpdir|
  queue = Queue.new
  jobs.each_with_index { |job, i| queue << [job, i] }
  queue.close
  mutex = Mutex.new
  workers = Array.new([options[:jobs], jobs.size].min) do
    Thread.new do
      while (item = queue.pop)
        job, i = item
        count = run_benchmark(options[:valgrind], job[:exe], job[:suite], job[:bench], iters,
                              File.join(tmpdir, "cg.out.#{i}"))
        mutex.synchronize { results[job[:key]] = count }
      end
    end
  end
  workers.each(&:join) # raises if a worker raised
end

zero = results.select { |_, v| v == 0 }.keys
unless zero.empty?
  puts "Error: zero instructions counted for:\n  #{zero.join("\n  ")}"
  puts 'Are the benchmarks built with DYNAMIX_BENCH_CACHEGRIND?'
  exit 1
end

# the exact counts are stored and compared
# they are divided by the number of iterations only for display
results = results.sort.to_h
data = { 'iters' => iters, 'instructions' => results }
json = JSON.pretty_generate(data) + "\n"

File.write(options[:output], json) if options[:output]

if options[:update]
  File.write(options[:baseline], json)
  puts "Baseline updated: #{options[:baseline]}"
end

baseline = nil
if File.exist?(options[:baseline])
  baseline_data = JSON.parse(File.read(options[:baseline]))
  if baseline_data['iters'] != iters
    puts "Error: baseline was created with iters=#{baseline_data['iters']}, results are with iters=#{iters}"
    exit 1
  end
  baseline = baseline_data['instructions']
end

per_op = ->(n) { fmt_count(n.to_f / iters) }

lines = ['## Cachegrind benchmark results', '']
lines << "Instructions per iteration. Regression threshold: #{fmt_percent(tolerance, 0)}."
lines << ''

regressions = []
if baseline.nil?
  lines << "No baseline found at `#{options[:baseline]}`. Nothing to compare to."
  lines << ''
  lines << '| Benchmark | Instructions |'
  lines << '|-----------|-------------:|'
  results.each { |k, v| lines << "| #{k} | #{per_op.(v)} |" }
else
  lines << '| Benchmark | Baseline | Current | Change | |'
  lines << '|-----------|---------:|--------:|-------:|-|'
  results.each do |k, v|
    b = baseline[k]
    if b.nil?
      lines << "| #{k} | | #{per_op.(v)} | | new |"
      next
    end
    change = b.zero? ? 0.0 : v.to_f / b - 1
    status = ''
    if change > tolerance
      status = 'regression'
      regressions << k
    elsif change < -tolerance
      status = 'improvement'
    end
    lines << "| #{k} | #{per_op.(b)} | #{per_op.(v)} | #{fmt_percent(change, 1, sign: true)} | #{status} |"
  end
  (baseline.keys - results.keys).sort.each do |k|
    lines << "| #{k} | #{per_op.(baseline[k])} | | | removed |"
  end
end

lines << ''
if !regressions.empty? && !options[:update]
  lines << "**#{regressions.size} regression(s)**. If they are expected, update the baseline (see `bench/cachegrind/README.md`)."
end
report = lines.join("\n") + "\n"

puts report
summary = ENV['GITHUB_STEP_SUMMARY']
File.write(summary, report, mode: 'a') if summary

exit(!regressions.empty? && !options[:update] ? 1 : 0)
