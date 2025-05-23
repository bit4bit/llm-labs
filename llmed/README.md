# LLMED

Single LLM Code editor.

Concepts:
* Source Code = This (there is not name yet)
* Application = Legacy Source Code
* Compiler = LLM

```ruby
set_llm provider: :openai, api_key: ENV['OPENAI_API_KEY'], model: 'gpt-4o'

application "hi world", language: '<HERE LANGUAGE>', output_file: "<HERE NAME>.rb" do
  context "main" do
    <<-LLM
        Application do print 'hi world!'.
    LLM
  end
end
```

# Installation

* `bundle3.1 install --path vendor/`

# Usage Development

`OPENAI_API_KEY=xxx rake llmed[examples/tictactoe.rb]`
