#!/usr/bin/env ruby
# frozen_string_literal: true

# Test user program compilation and verification
# Usage: ./tools/test-programs.rb

require 'fileutils'

class TestRunner
  GREEN = "\e[0;32m"
  RED = "\e[0;31m"
  YELLOW = "\e[1;33m"
  NC = "\e[0m"

  attr_reader :tests_passed, :tests_failed

  def initialize
    @tests_passed = 0
    @tests_failed = 0
    @project_root = File.expand_path('..', __dir__)
  end

  def pass(message)
    puts "#{GREEN}✓ PASS#{NC}: #{message}"
    @tests_passed += 1
  end

  def fail(message)
    puts "#{RED}✗ FAIL#{NC}: #{message}"
    @tests_failed += 1
  end

  def info(message)
    puts "#{YELLOW}ℹ INFO#{NC}: #{message}"
  end

  def file_exists?(path)
    File.file?(File.join(@project_root, path))
  end

  def run_test(name)
    puts
    puts "Test #{@tests_passed + @tests_failed + 1}: #{name}"
    puts '-' * 40
    yield
  end

  def summary
    puts
    puts '=' * 44
    puts 'Test Results'
    puts '=' * 44
    puts "Passed: #{GREEN}#{@tests_passed}#{NC}"
    puts "Failed: #{RED}#{@tests_failed}#{NC}"
    puts "Total:  #{@tests_passed + @tests_failed}"
    puts

    if @tests_failed.zero?
      puts "#{GREEN}All tests passed! ✨#{NC}"
      puts
      puts 'Next steps:'
      puts '  1. Include programs/generated/hello_bin.c in your kernel'
      puts '  2. Copy hello_bin[] to user space (0x40000000)'
      puts '  3. Create and run the process'
      exit 0
    else
      puts "#{RED}Some tests failed.#{NC} Please review the errors above."
      exit 1
    end
  end
end

runner = TestRunner.new

puts '=' * 44
puts 'MiniOS User Programs - Test Suite'
puts '=' * 44

# Test 1: Check required files exist
runner.run_test('Check required files exist') do
  runner.pass('programs/lib/syscall.h exists') if runner.file_exists?('programs/lib/syscall.h')
  runner.fail('programs/lib/syscall.h missing') unless runner.file_exists?('programs/lib/syscall.h')

  runner.pass('programs/lib/stdint.h exists') if runner.file_exists?('programs/lib/stdint.h')
  runner.fail('programs/lib/stdint.h missing') unless runner.file_exists?('programs/lib/stdint.h')

  runner.pass('programs/user.ld exists') if runner.file_exists?('programs/user.ld')
  runner.fail('programs/user.ld missing') unless runner.file_exists?('programs/user.ld')

  runner.pass('tools/bin2c.rb exists') if runner.file_exists?('tools/bin2c.rb')
  runner.fail('tools/bin2c.rb missing') unless runner.file_exists?('tools/bin2c.rb')

  runner.pass('programs/src directory exists') if Dir.exist?(File.join(runner.instance_variable_get(:@project_root),
                                                                       'programs/src'))
  runner.fail('programs/src directory missing') unless Dir.exist?(File.join(
                                                                    runner.instance_variable_get(:@project_root), 'programs/src'
                                                                  ))
end

# Test 2: Build user programs
runner.run_test('Build user programs') do
  result = system('ruby ../tools/build-programs.rb --clean',
                  chdir: File.join(runner.instance_variable_get(:@project_root), 'programs'))
  runner.pass('Clean successful') if result
  runner.fail('Clean failed') unless result

  result = system('ruby ../tools/build-programs.rb --all',
                  chdir: File.join(runner.instance_variable_get(:@project_root), 'programs'))
  runner.pass('Build successful') if result
  runner.fail('Build failed') unless result
end

# Test 3: Verify binary output
runner.run_test('Verify binary output') do
  if runner.file_exists?('programs/src/hello/hello.bin')
    runner.pass('hello.bin created')

    size = File.size(File.join(runner.instance_variable_get(:@project_root), 'programs/src/hello/hello.bin'))
    runner.info "Binary size: #{size} bytes"

    if size < 1024
      runner.pass('Binary size is reasonable (< 1KB)')
    else
      runner.fail('Binary size too large (>= 1KB)')
    end

    if size > 0
      runner.pass('Binary is not empty')
    else
      runner.fail('Binary is empty')
    end
  else
    runner.fail('hello.bin not created')
  end
end

# Test 4: Generate C arrays
runner.run_test('Generate C arrays') do
  result = system('ruby ../tools/build-programs.rb --generated',
                  chdir: File.join(runner.instance_variable_get(:@project_root), 'programs'))
  runner.pass('Generated C arrays') if result
  runner.fail('Failed to generate C arrays') unless result

  if runner.file_exists?('programs/generated/hello_bin.c')
    runner.pass('hello_bin.c created')

    content = File.read(File.join(runner.instance_variable_get(:@project_root), 'programs/generated/hello_bin.c'))

    if content.include?('uint8_t hello[]')
      runner.pass('Contains hello array declaration')
    else
      runner.fail('Missing hello array declaration')
    end

    if content.include?('uint32_t hello_size')
      runner.pass('Contains hello_size variable')
    else
      runner.fail('Missing hello_size variable')
    end
  else
    runner.fail('hello_bin.c not created')
  end
end

# Test 5: Verify binary content
runner.run_test('Verify binary content') do
  bin_path = File.join(runner.instance_variable_get(:@project_root), 'programs/src/hello/hello.bin')

  if File.file?(bin_path)
    content = File.read(bin_path, mode: 'rb')

    if content.include?("\xcd\x80".b)
      runner.pass('Binary contains syscall instruction (int 0x80)')
    else
      runner.fail('Binary missing syscall instruction')
    end

    if content.include?('Hello from C program'.encode('ASCII-8BIT'))
      runner.pass('Binary contains expected message string')
    else
      runner.fail('Binary missing expected message string')
    end

    if content.bytes.first != 0
      runner.pass('Binary starts with non-zero byte')
    else
      runner.fail('Binary starts with zero (might be empty)')
    end
  end
end

# Test 6: Test bin2c.rb tool
runner.run_test('Test bin2c.rb tool') do
  runner.pass('bin2c.rb is executable') if File.executable?(File.join(runner.instance_variable_get(:@project_root),
                                                                      'tools/bin2c.rb'))

  temp_bin = '/tmp/test_minios.bin'
  temp_c = '/tmp/test_minios_array.c'

  begin
    File.write(temp_bin, [0x48, 0x65, 0x6C, 0x6C, 0x6F].pack('C*'))

    cmd = "ruby #{File.join(runner.instance_variable_get(:@project_root),
                            'tools/bin2c.rb')} #{temp_bin} test_array > #{temp_c} 2>/dev/null"
    result = system(cmd)

    if result
      runner.pass('bin2c.rb can convert binary to C array')

      if File.read(temp_c).include?('uint8_t test_array[]')
        runner.pass('Generated array has correct name')
      else
        runner.fail('Generated array has incorrect name')
      end

      if File.read(temp_c).include?('0x48')
        runner.pass('Generated array contains hex values')
      else
        runner.fail('Generated array missing hex values')
      end
    else
      runner.fail('bin2c.rb failed to convert binary')
    end
  ensure
    File.delete(temp_bin) if File.exist?(temp_bin)
    File.delete(temp_c) if File.exist?(temp_c)
  end
end

# Test 7: Check syscall.h syntax
runner.run_test('Check syscall.h syntax') do
  temp_c = File.join(runner.instance_variable_get(:@project_root), 'tmp_test_syscall.c')
  temp_o = File.join(runner.instance_variable_get(:@project_root), 'tmp_test_syscall.o')

  begin
    File.write(temp_c, <<~C)
      #include "programs/lib/syscall.h"

      void _start(void) {
          write(1, "test", 4);
          exit(0);
      }
    C

    cmd = "cd #{runner.instance_variable_get(:@project_root)} && gcc -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -fno-builtin -nostdinc -I. -c tmp_test_syscall.c -o tmp_test_syscall.o 2>/dev/null"
    result = system(cmd)

    if result
      runner.pass('syscall.h compiles without errors')
    else
      runner.fail('syscall.h has compilation errors')
    end
  ensure
    File.delete(temp_c) if File.exist?(temp_c)
    File.delete(temp_o) if File.exist?(temp_o)
  end
end

runner.summary
