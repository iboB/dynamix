#!/usr/bin/env ruby
# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
# run benchmarks with cachegrind and compare the instruction counts with a baseline.
# see README.md
#
require 'etc'
require 'json'
require 'open3'
require 'optparse'
require 'tmpdir'

def list_benchmarks(exe)
  out, status = Open3.capture2(exe, '--list')
  raise "FAILED: #{exe} --list" unless status.success?
  ret = []
  suite = nil
  out.each_line do |line|
    if line.start_with?(' ' * 4)
      ret << [suite, line.strip]
    elsif line.start_with?(' ' * 2)
      suite = line.strip[...-1]
    end
  end
  ret
end

def run_benchmark(valgrind, exe, suite, bench, iters, tmp_file)
  cmd = [
    valgrind, '--tool=cachegrind', '--cache-sim=no', '--instr-at-start=no',
    "--cachegrind-out-file=#{tmp_file}",
    exe, "--run-suite=#{suite}", "--run-only=#{bench}", "--iters=#{iters}", '--samples=1'
  ]
  out, err, status = Open3.capture3(*cmd)
  raise "FAILED: #{cmd}\n#{out}\n#{err}" unless status.success?
  File.foreach(tmp_file) do |line|
    m = line.match(/^summary:\s+(\d+)/)
    return (m[1].to_f / iters).round if m
  end
  raise "no summary in cachegrind output for #{exe} #{suite}/#{bench}"
end

options = {
  valgrind: 'valgrind',
  baseline: nil,
  output: nil,
  update_baseline: false,
  tolerance: 0.02,
  iters: 1024,
  summary: nil,
  fail_reg: true,
  fail_imp: false,
  fail_new: false,
  fail_rem: false,
}

parser = OptionParser.new { |opts|
  opts.banner = "Usage #{$PROGRAM_NAME} [options] exe [exe...]\nRun benchmarks with cachegrind"
  opts.on('--valgrind PATH', 'valgrind executable') { options[:valgrind] = _1 }
  opts.on('--baseline FILE', 'baseline json file (required)') { options[:baseline] = _1 }
  opts.on('--output FILE', 'write the results to file') { options[:output] = _1 }
  opts.on('--update-baseline', 'overwrite the baseline with the results') { options[:update_baseline] = _1 }
  opts.on('--tolerance F', Float, 'relative change to count as regression (default 0.02)') { options[:tolerance] = _1 }
  opts.on('--iters N', Integer, 'itetrations for each benchmark (default 1024)') { options[:iters] = _1 }
  opts.on('--summary FILE', 'write the summary to file') { options[:summary] = _1 }
  opts.on('--fail-on-regression', 'fail on a regression') { options[:fail_reg] = _1 }
  opts.on('--fail-on-improvement', 'fail on an improvement') { options[:fail_imp] = _1 }
  opts.on('--fail-on-new', 'fail on an unknown benchmark') { options[:fail_new] = _1 }
  opts.on('--fail-on-removed', 'fail on a missing benchmark') { options[:fail_rem] = _1 }
}
exes = parser.parse(ARGV)
baseline_file = options[:baseline]
if !baseline_file || exes.empty?
  puts parser
  exit 1
end

puts "Running benchmarks with cachegrind..."

benchmarks = exes.flat_map { |exe|
  list_benchmarks(exe).map { [exe, _1, _2] }
}

results = Dir.mktmpdir do |tmpdir|
  benchmarks.map.with_index { |(exe, suite, bench), i|
    [
      "#{File.basename(exe, '.*')}/#{suite}/#{bench}", # key
      run_benchmark(options[:valgrind], exe, suite, bench, options[:iters], File.join(tmpdir, "cg.out.#{i}")) # count
    ]
  }.to_h
end

json = JSON.pretty_generate(results)
if options[:output]
  File.write(options[:output], json)
else
  puts json
end

if options[:update_baseline]
  File.write(baseline_file, json)
  puts "Updated baseline: #{baseline_file}"
  exit
end

tolerance = options[:tolerance]
puts "Comparing results with baseline: #{baseline_file}"
baseline = JSON.parse(File.read(baseline_file))

output = ['## Cachegrind benchmarks results', '']
output << "Instructions per iteration. Regression threshold: #{tolerance}"

new = []
regressions = []
improvements = []

output << '| Benchmark | Baseline | Current | Change | |'
output << '|-----------|---------:|--------:|-------:|-|'
output += results.map { |k, v|
  b = baseline[k]
  if !b
    new << k
    "| #{k} | | #{v} | | new |"
  else
    change = v.to_f / b - 1
    status = ''
    if change > tolerance
      status = 'regression'
      regressions << k
    elsif change < -tolerance
      status = 'improvement'
      improvements << k
    end
    "| #{k} | #{b} | #{v} | #{change.round(2)} | #{status} |"
  end
}

removed = (baseline.keys - results.keys)
output += removed.map { |k|
  "| #{k} | #{baseline[k]} | | | removed |"
}

output << ''

def output_list(out, name, list, fail)
  return 0 if list.empty?
  out << "### #{name}"
  out << ''
  list.each { out <<  "* #{_1}" }
  out << ''
  return fail ? 1 : 0
end

fail = output_list(output, "Regressions", regressions, options[:fail_reg])
fail += output_list(output, "Improvements", improvements, options[:fail_imp])
fail += output_list(output, "New", new, options[:fail_new])
fail += output_list(output, "Removed", removed, options[:fail_rem])

output = output.join("\n")
if options[:summary]
  File.write(options[:summary], output)
else
  puts output
end

exit(fail)
