#!/usr/bin/env ruby
# frozen_string_literal: true

# Convert binary file to C array
# Usage: ./bin2c.rb <input.bin> <array_name>

def bin2c(input_file, array_name)
  unless File.file?(input_file)
    warn "Error: Input file '#{input_file}' not found"
    exit 1
  end

  file_size = File.size(input_file)
  data = File.read(input_file, mode: 'rb')

  puts "/* Auto-generated from #{input_file} */"
  puts "/* Size: #{file_size} bytes */"
  puts
  puts '#include <stdint.h>'
  puts
  puts "uint8_t #{array_name}[] = {"

  bytes_per_line = 16
  data.bytes.each_slice(bytes_per_line).each_with_index do |slice, index|
    hex_values = slice.map { |b| format('0x%02X', b) }.join(', ')
    trailing_comma = (index + 1) * bytes_per_line < data.bytes.size ? ',' : ''
    puts "    #{hex_values}#{trailing_comma}"
  end

  puts '};'
  puts
  puts "uint32_t #{array_name}_size = sizeof(#{array_name});"
end

if ARGV.size != 2
  warn 'Usage: bin2c.rb <input.bin> <array_name>'
  warn 'Example: bin2c.rb hello.bin hello_bin'
  exit 1
end

input_file = ARGV[0]
array_name = ARGV[1]

bin2c(input_file, array_name)
