Gem::Specification.new do |s|
  s.name        = 'llmed'
  s.version     = '0.1.1'
  s.licenses    = ['GPLv3']
  s.description = "LLM Execution Development"
  s.summary     = "Use this 'compiler' to build software using LLMs in a controlled way. In classical terms, the LLM is the compiler, the context description is the programming language, and the generated output is the binary."
  s.authors     = ["Jovany Leandro G.C"]
  s.email       = 'bit4bit@riseup.net'
  s.files       = ["lib/llmed.rb"]
  s.homepage    = "https://github.com/bit4bit/llm-labs/tree/main/llmed",
  s.metadata    = { "source_code_uri" => "https://github.com/bit4bit/llm-labs" }
  
  s.bindir = 'exe'
  s.executables << 'llmed'

  s.add_dependency "langchainrb", "~> 0.19.5"
  s.add_dependency "ruby-openai", "~> 8.1"

  s.add_development_dependency 'rspec'
  s.add_development_dependency 'rubocop'
end
