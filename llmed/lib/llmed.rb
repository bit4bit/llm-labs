require 'pp'
require 'langchain'
require 'pathname'
require 'fileutils'
require 'forwardable'

Langchain.logger.level = Logger::ERROR

class LLMed
  extend Forwardable

  class Context
    attr_reader :message, :name

    def initialize(name:)
      @name = name
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
Todo el codigo fuente se genera en un unico archivo.
Siempre adicionas el comentario de codigo:
 @<LLMED-COMPILED>
 @<#{Time.now}>
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

    def initialize(name:, language:, output_file:, block:)
      raise "required language" if language.nil?

      @name = name
      @output_file = output_file
      @language = language
      @block = block
      @contexts = []
    end

    def context(name, &block)
      ctx = Context.new(name: name)
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
      Pathname.new(output_dir) + @output_file
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
    @app = Application.new(name: name, language: @configuration.language(language), output_file: output_file, block: block)
    @applications << @app
  end

  def compile(output_dir:)
    llm = @configuration.llm()

    @applications.each do |app|
      messages = [
        {role: "system", content: @configuration.prompt(language: app.language)},
      ]
      app.evaluate
      app.contexts.each do |ctx|
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
    content.gsub('```', '').gsub(/^(ruby|python|elixir|c)/, '')
  end

  def write_output(app, output_dir, output)
    output_file = app.output_file(output_dir)
    FileUtils.mkdir_p(File.dirname(output_file))

    File.write(output_file, output)

    @logger.info("APPLICATION #{app.name} OUTPUT FILE #{output_file}")
  end
end
