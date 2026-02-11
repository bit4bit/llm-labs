#!/usr/bin/env ruby
# frozen_string_literal: true

# Build user programs for MiniOS Minimax.
# Usage: ./build-programs.rb [--clean] [--programs] [--generated] [--all]

require 'fileutils'

SCRIPT_DIR = File.dirname(File.expand_path(__FILE__))
PROGRAMS_DIR = File.join(SCRIPT_DIR, '..', 'programs')
SRC_DIR = File.join(PROGRAMS_DIR, 'src')
GENERATED_DIR = File.join(PROGRAMS_DIR, 'generated')
LIB_DIR = File.join(PROGRAMS_DIR, 'lib')

CFLAGS = %W[
  -std=c18
  -m32
  -ffreestanding
  -nostdlib
  -fno-pie
  -fno-stack-protector
  -fno-builtin
  -nostdinc
  -O2
  -Wall
  -Wextra
  -I#{LIB_DIR}
].join(' ')

LDFLAGS = "-m elf_i386 -T #{File.join(PROGRAMS_DIR, 'user.ld')} --no-relax"

def find_programs
  programs = []
  return programs unless Dir.exist?(SRC_DIR)

  Dir.foreach(SRC_DIR) do |entry|
    next unless File.directory?(File.join(SRC_DIR, entry))
    next if ['.', '..'].include?(entry)

    c_file = File.join(SRC_DIR, entry, "#{entry}.c")
    programs << entry if File.file?(c_file)
  end

  programs.sort
end

def build_program(prog)
  src_file = File.join(SRC_DIR, prog, "#{prog}.c")
  obj_file = File.join(SRC_DIR, prog, "#{prog}.o")
  bin_file = File.join(SRC_DIR, prog, "#{prog}.bin")

  puts "[BUILD] #{prog}/#{prog}.c -> #{prog}/#{prog}.bin"

  unless File.file?(src_file)
    puts "  ERROR: #{src_file} not found"
    return false
  end

  FileUtils.mkdir_p(File.dirname(obj_file))

  cmd_compile = "gcc #{CFLAGS} -c #{src_file} -o #{obj_file}"
  result = `#{cmd_compile} 2>&1`
  unless $?.success?
    puts "  COMPILE ERROR: #{result}"
    return false
  end

  cmd_link = "ld #{LDFLAGS} -o #{bin_file} #{obj_file}"
  result = `#{cmd_link} 2>&1`
  unless $?.success?
    puts "  LINK ERROR: #{result}"
    return false
  end

  puts "  OK: #{bin_file}"
  true
end

def generate_c_array(prog)
  bin_file = File.join(SRC_DIR, prog, "#{prog}.bin")
  gen_file = File.join(GENERATED_DIR, "#{prog}_bin.c")

  puts "[GEN] #{prog}_bin.c"

  cmd = "ruby #{File.join(SCRIPT_DIR, 'bin2c.rb')} #{bin_file} #{prog}_bin"
  result = `#{cmd} 2>&1`
  unless $?.success?
    puts "  ERROR: #{result}"
    return false
  end

  File.write(gen_file, result)
  puts "  OK: #{gen_file}"
  true
end

def clean(programs)
  puts '[CLEAN]'

  FileUtils.rm_rf(GENERATED_DIR)

  programs.each do |prog|
    %w[.o .bin].each do |ext|
      f = File.join(SRC_DIR, prog, "#{prog}#{ext}")
      if File.exist?(f)
        File.delete(f)
        puts "  Removed: #{f}"
      end
    end
  end

  puts '  Done.'
end

def main
  programs = find_programs

  if programs.empty?
    puts 'No programs found in src/'
    exit 1
  end

  puts 'MiniOS User Programs Builder'
  puts "Programs found: #{programs.join(', ')}"
  puts

  clean_flag = ARGV.include?('--clean')
  all_flag = ARGV.include?('--all')
  programs_flag = ARGV.include?('--programs')
  generated_flag = ARGV.include?('--generated')

  if clean_flag
    clean(programs)
    exit 0
  end

  do_programs = all_flag || programs_flag || !generated_flag
  do_generated = all_flag || generated_flag

  FileUtils.mkdir_p(GENERATED_DIR)

  if do_programs
    programs.each do |prog|
      exit 1 unless build_program(prog)
    end
  end

  if do_generated
    programs.each do |prog|
      exit 1 unless generate_c_array(prog)
    end
  end

  puts
  puts 'Done!'
  0
end

exit(main)
