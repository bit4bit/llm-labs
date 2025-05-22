require 'pp'
require 'langchain'
require 'pathname'
require 'fileutils'

Langchain.logger.level = Logger::ERROR

class LLMed
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
  end

  class Application
    attr_reader :contexts, :name, :language

    # TODO: add provider and model
    def initialize(name:, language:, output_file:)
      @name = name
      @output_file = output_file
      @language = language
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

    def output_file(output_dir)
      Pathname.new(output_dir) + @output_file
    end
  end

  def initialize(logger:)
    @logger = logger
    @applications = []
  end

  def eval_source(code)
    self.instance_eval(code)
  end

  def application(name, language:, output_file:, &block)
    @app = Application.new(name: name, language: language, output_file: output_file)
    @app.instance_eval(&block)
    @applications << @app
  end

  def compile(output_dir:)
    llm = Langchain::LLM::OpenAI.new(
      api_key: ENV["OPENAI_API_KEY"],
      default_options: { temperature: 0.7, chat_model: "gpt-4o" }
    )

    @applications.each do |app|
      messages = [
        {role: "system", content: "Eres desarrollador de software y solo conoces del lenguage de programacion #{app.language}. La respuesta no debe contener texto adicional al codigo fuente generado. Siempre adicionas el comentario '@LLM-ASSISTED' "},
      ]

      app.contexts.each do |ctx|
        messages << {role: "user", content: ctx.message}
      end

      llm_response = llm.chat(messages: messages)
      response = llm_response.chat_completion
      @logger.info("APPLICATION #{app.name} TOTAL TOKENS #{llm_response.total_tokens}")
      write_output(app, output_dir, source_code(response))
      check_syntax(app, output_dir, source_language(response))
    end
  end

  private
  def source_language(content)
    content.gsub('```', '')[/^(.+)\n/]
  end

  def source_code(content)
    content.gsub('```', '').gsub(/^(ruby|python)/, '')
  end

  def write_output(app, output_dir, output)
    output_file = app.output_file(output_dir)
    FileUtils.mkdir_p(File.dirname(output_file))

    File.write(output_file, output)

    @logger.info("APPLICATION #{app.name} OUTPUT FILE #{output_file}")
  end

  def check_syntax(app, output_dir, language)
    source_file = app.output_file(output_dir)

    case language
    when 'ruby'
      system("ruby -c #{source_file} > /dev/null")
      unless $?.success?
        raise "invalid generated ruby file #{output_file}"
      else
        @logger.info("SYNTAX OK")
      end
    when 'python'
      system("python3 -m py_compile #{source_file}")
      unless $?.success?
        raise "invalid generated Python file #{output_file}"
      else
        @logger.info("SYNTAX OK")
      end
    end
  end
end
