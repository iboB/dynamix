# c2md - v1.0 - public domain
# authored in 2016 by Borislav Stanimirov

# Used to generate dox files from cpp files

if ARGV.length == 0 || ARGV[0] == '--help' || ARGV[0] == '-?'
  puts 'c2md'
  puts 'Generate markdown doc from a C-like language'
  puts
  puts 'Usage:'
  puts 'ruby c2md.rb -i <source files> -t <templates>'
  puts
  exit(0)
end

if ARGV[0] == '--version' || ARGV[0] == '-v'
  puts 'c2md version 1.0.0'
  puts
  exit(0)
end

if ARGV[0] != '-i'
  puts 'Missing source files'
  puts
  exit(1)
end

inputs = []
ai = 1
1.upto(ARGV.length - 1) do |i|
  ai += 1
  break if ARGV[i] == '-t'
  inputs << ARGV[i]
end

if inputs.empty?
  puts 'Missing source files'
  puts
  exit(1)
end

templates = []
ai.upto(ARGV.length - 1) do |i|
  templates << ARGV[i]
end

if templates.empty?
  puts 'Missing templates'
  puts
  exit(1)
end

blocks = {}

inputs.each do |fname|
  input = File.open(fname, 'r')

  cur_block_name = ''
  cur_block = []
  is_code = false
  is_text = false

  input.each_line do |line|
    if cur_block_name.empty?
      cur_block_name = line[3..-2] if line.start_with?('//[')
    else
      if line.start_with?('//]')
        cur_block << '```' if is_code
        blocks[cur_block_name.to_sym] = cur_block
        cur_block_name = ''
        cur_block = []
        is_code = false
        is_text = false
      elsif line.start_with?('/*`')
        cur_block << '```' if is_code
        is_code = false
        is_text = true
      elsif line.start_with?('*/')
        is_text = false
      elsif line.start_with?('//` ')
        cur_block << '```' if is_code
        is_code = false
        cur_block << line[4..-2].rstrip
      elsif line.start_with?('//`')
        cur_block << '```' if is_code
        is_code = false
        cur_block << line[3..-2].rstrip
      elsif is_text || is_code
        cur_block << line.rstrip
      else
        if !line.strip.empty?
          cur_block << ''
          cur_block << '```'
          cur_block << line.rstrip
          is_code = true
        end
      end
    end
  end
end

text_blocks = {}
blocks.each do |k, v|
  text_blocks[k] = v.join("\n")
end

expand_blocks = {}
text_blocks.each do |k, v|
  expand_blocks[k] = v % text_blocks
end

templates.each do |tname|
  fname = tname.chomp('.template')
  template_src = File.open(tname, 'r').read
  File.open(fname, 'w').write(template_src % expand_blocks)
end
