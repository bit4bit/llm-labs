# Copyright 2025 Jovany Leandro G.C <bit4bit@riseup.net>
# frozen_string_literal: true

require 'llmed'
require 'logger'
require 'stringio'

describe LLMed do
  it "configuration" do
    logger = Logger.new(STDOUT)
    llmed = LLMed.new(logger: logger)
    
    llmed.set_language 'ruby'
    llmed.set_llm provider: :openai, api_key: 'key', model: 'model'
  end

  context "ruby application" do
    it "compile application skip context" do
      logger = Logger.new(STDOUT)
      llmed = LLMed.new(logger: logger)
      llmed.set_llm(provider: :openai, api_key: ENV['OPENAI_API_KEY'], model: 'gpt-4o')
      llmed.set_language 'ruby'
      fake = StringIO.new
      llmed.application "demo", output_file: fake do
        context("main", skip: true) { from_file('./spec/hiworld.cllmed') }
      end
      llmed.compile(output_dir: '/tmp')

      
      expect(fake.string).not_to including("hola mundo")
    end

    it "compile application to file" do
      logger = Logger.new(STDOUT)
      llmed = LLMed.new(logger: logger)
      llmed.set_llm(provider: :openai, api_key: ENV['OPENAI_API_KEY'], model: 'gpt-4o')
      llmed.set_language 'ruby'
      fake = StringIO.new
      llmed.application "demo", output_file: fake do
        context("main") { from_file('./spec/hiworld.cllmed') }
      end
      llmed.compile(output_dir: '/tmp')

      
      expect(fake.string).to including("puts 'hola mundo'")
    end

    it "compile application connecting applications through output" do
      tempfile = %x{mktemp}.chomp
      tempfile_bye = %x{mktemp}.chomp
      logger = Logger.new(STDOUT)
      llmed = LLMed.new(logger: logger)
      llmed.set_llm(provider: :openai, api_key: ENV['OPENAI_API_KEY'], model: 'gpt-4o')
      llmed.set_language 'ruby'
      
      llmed.application "main", output_file: tempfile do
        context "main" do
          llm <<-LLM
        Imprimir mensaje 'hola mundo'.
        LLM
        end
      end

      llmed.application "demo", output_file: tempfile_bye do
        context("main") { from_source_code(tempfile) }
        context("adicionar despedida") {
          <<-LLM
          Adicionar mensaje de despedida 'bye mundo'.
          LLM
        }
      end

      llmed.compile(output_dir: '/tmp')
      
      expect(File.read tempfile_bye).to including("puts 'hola mundo'")
      expect(File.read tempfile_bye).to including("puts 'bye mundo'")
    end

    it "compile application including context" do
      tempfile = %x{mktemp}.chomp
      logger = Logger.new(STDOUT)
      llmed = LLMed.new(logger: logger)
      llmed.set_llm(provider: :openai, api_key: ENV['OPENAI_API_KEY'], model: 'gpt-4o')
      llmed.set_language 'ruby'
      llmed.application "demo", output_file: tempfile do
        context("main") { from_file('./spec/hiworld.cllmed') }
      end

      llmed.compile(output_dir: '/tmp')
      
      expect(File.read tempfile).to including("puts 'hola mundo'")
    end

    it "compile application with implicit language" do
      tempfile = %x{mktemp}.chomp
      logger = Logger.new(STDOUT)
      llmed = LLMed.new(logger: logger)
      llmed.set_llm(provider: :openai, api_key: ENV['OPENAI_API_KEY'], model: 'gpt-4o')
      llmed.set_language 'ruby'
      llmed.application "demo", output_file: tempfile do
        context "main" do
          llm <<-LLM
        Codigo que imprima 'hola mundo'.
        LLM
        end
      end

      llmed.compile(output_dir: '/tmp')
      
      expect(File.read tempfile).to including("puts 'hola mundo'")
    end

    it "compile application with explicit language" do
      tempfile = %x{mktemp}.chomp
      logger = Logger.new(STDOUT)
      llmed = LLMed.new(logger: logger)
      llmed.set_llm(provider: :openai, api_key: ENV['OPENAI_API_KEY'], model: 'gpt-4o')
      llmed.application "demo", language: 'ruby', output_file: tempfile do
        context "main" do
          llm <<-LLM
        Codigo que imprima 'hola mundo'.
        LLM
        end
      end

      llmed.compile(output_dir: '/tmp')
      
      expect(File.read tempfile).to including("puts 'hola mundo'")
    end

    it "compile application from string" do
      tempfile = %x{mktemp}.chomp
      logger = Logger.new(STDOUT)
      llmed = LLMed.new(logger: logger)
      llmed.set_llm(provider: :openai, api_key: ENV['OPENAI_API_KEY'], model: 'gpt-4o')
      llmed.eval_source <<-SOURCE
    application "demo", language: 'ruby', output_file: '#{tempfile}' do
      context "main" do
        llm <<-LLM
        Codigo ruby que imprima 'hola mundo'.
        LLM
      end
    end
    SOURCE

      llmed.compile(output_dir: '/tmp')
      
      expect(File.read tempfile).to including("puts 'hola mundo'")
    end
  end

  context "python application" do
    it "compile application" do
      tempfile = %x{mktemp}.chomp
      logger = Logger.new(STDOUT)
      llmed = LLMed.new(logger: logger)
      llmed.set_llm(provider: :openai, api_key: ENV['OPENAI_API_KEY'], model: 'gpt-4o')
      llmed.application "demo", language: 'python', output_file: tempfile do
        context "main" do
          llm <<-LLM
        Codigo que imprima 'hola mundo'.
        LLM
        end
      end

      llmed.compile(output_dir: '/tmp')
      
      expect(File.read tempfile).to including("print('hola mundo')")
    end

    it "compile application from string" do
      tempfile = %x{mktemp}.chomp
      logger = Logger.new(STDOUT)
      llmed = LLMed.new(logger: logger)
      llmed.set_llm(provider: :openai, api_key: ENV['OPENAI_API_KEY'], model: 'gpt-4o')
      llmed.eval_source <<-SOURCE
    application "demo", language: 'python', output_file: '#{tempfile}' do
      context "main" do
        llm <<-LLM
        Codigo python que imprima 'hola mundo'.
        LLM
      end
    end
    SOURCE

      llmed.compile(output_dir: '/tmp')
      
      expect(File.read tempfile).to including("print('hola mundo')")
    end
  end

  context "multiple applications" do
    it "ruby and python" do
      tempfile_ruby = %x{mktemp}.chomp
      tempfile_python = %x{mktemp}.chomp
      logger = Logger.new(STDOUT)
      llmed = LLMed.new(logger: logger)
      llmed.set_llm(provider: :openai, api_key: ENV['OPENAI_API_KEY'], model: 'gpt-4o')
      llmed.application "demo", language: 'ruby', output_file: tempfile_ruby do
        context "main" do
          llm <<-LLM
        Codigo que imprima 'hola mundo'.
        LLM
        end
      end

      llmed.application "demo python", language: 'python', output_file: tempfile_python do
        context "main" do
          llm <<-LLM
        Codigo que imprima 'hola mundo'.
        LLM
        end
      end

      llmed.compile(output_dir: '/tmp')
      
      expect(File.read tempfile_ruby).to including("puts 'hola mundo'")
      expect(File.read tempfile_python).to including("print('hola mundo')")
    end
  end
end
