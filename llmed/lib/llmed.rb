# Copyright 2025 Jovany Leandro G.C <bit4bit@riseup.net>
# frozen_string_literal: true

require 'pp'
require 'langchain'
require 'pathname'
require 'fileutils'
require 'forwardable'

Langchain.logger.level = Logger::ERROR

class LLMed
  extend Forwardable

  class Context
    attr_reader :name

    def initialize(name:, options: {})
      @name = name
      @skip = options[:skip] || false
    end

    def skip?
      @skip
    end

    def message
      "#{@name}\n\n#{@message}"
    end

    def llm(message)
      @message = message
    end

    def message?
      not (@message.nil? || @message.empty?)
    end


    def from_file(path)
      File.read(path)
    end

    def from_source_code(path)
      code = File.read(path)
      "Dado el codigo fuente: #{code}\n\n\n"
    end
  end

  class Configuration
    def initialize
      @prompt = Langchain::Prompt::PromptTemplate.new(template: "
Eres desarrollador de software y solo conoces del lenguage de programacion {language}.
La respuesta no debe contener texto adicional al codigo fuente generado.
Todo el codigo fuente se genera en un unico archivo y debes asegurarte de que se ejecute correctamente desde el primer intento.
Siempre adicionas el comentario de codigo correctamente escapado LLMED-COMPILED.

", input_variables: ["language"])
    end

    def prompt(language:)
      @prompt.format(language: language)
    end

    def set_prompt(prompt)
      @prompt = Langchain::Prompt::PromptTemplate.new(template: prompt, input_variables: ["language"])
    end

    def set_language(language)
      @language = language
    end

    def set_llm(provider:, api_key:, model:)
      @provider = provider
      @provider_api_key = api_key
      @provider_model = model
    end

    def language(main)
      lang = main || @language
      raise "Please assign a language to the application or general with the function set_languag" if lang.nil?
      lang
    end

    def llm()
      case @provider
      when :openai
        Langchain::LLM::OpenAI.new(
          api_key: @provider_api_key,
          default_options: { temperature: 0.7, chat_model: @provider_model}
        )
      when nil
        raise "Please set the provider with `set_llm(provider, api_key, model)`"
      else
        raise "not implemented provider #{@provider}"
      end
    end
  end

  class Application
    attr_reader :contexts, :name, :language

    def initialize(name:, language:, output_file:, block:, logger:)
      raise "required language" if language.nil?

      @name = name
      @output_file = output_file
      @language = language
      @block = block
      @contexts = []
      @logger = logger
    end

    def context(name, **opts, &block)
      ctx = Context.new(name: name, options: opts)
      output = ctx.instance_eval(&block)
      unless ctx.message?
        ctx.llm(output)
      end

      @contexts << ctx
    end

    def evaluate
      self.instance_eval(&@block)
    end

    def output_file(output_dir)
      if @output_file.respond_to? :write
        yield @output_file
      else
        path = Pathname.new(output_dir) + @output_file
        FileUtils.mkdir_p(File.dirname(path))

        @logger.info("APPLICATION #{@name} OUTPUT FILE #{path}")

        File.open(path, 'w') do |file|
          yield file
        end
      end
    end
  end

  def initialize(logger:)
    @logger = logger
    @applications = []
    @configuration = Configuration.new()
  end

  def eval_source(code)
    self.instance_eval(code)
  end

  # changes default language
  def_delegator :@configuration, :set_language, :set_language
  # changes default llm
  def_delegator :@configuration, :set_llm, :set_llm
  # changes default prompt
  def_delegator :@configuration, :set_prompt, :set_prompt

  def application(name, language: nil, output_file:, &block)
    @app = Application.new(name: name, language: @configuration.language(language), output_file: output_file, block: block, logger: @logger)
    @applications << @app
  end

  def compile(output_dir:)
    @applications.each do |app|
      llm = @configuration.llm()

      messages = [
        {role: "system", content: @configuration.prompt(language: app.language)},
      ]
      app.evaluate
      app.contexts.each do |ctx|
        next if ctx.skip?
        messages << {role: "user", content: ctx.message}
      end

      llm_response = llm.chat(messages: messages)
      response = llm_response.chat_completion
      @logger.info("APPLICATION #{app.name} TOTAL TOKENS #{llm_response.total_tokens}")
      write_output(app, output_dir, source_code(response))
    end
  end

  private
  def source_code(content)
    # TODO: by provider?
    content.gsub('```', '').sub(/^(ruby|python(\d*)|elixir|c(pp)?|perl|bash)/, '')
  end

  def write_output(app, output_dir, output)
    app.output_file(output_dir) do |file|
      file.write(output)
    end
  end
end
