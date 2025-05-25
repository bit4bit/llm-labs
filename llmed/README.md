# LLMED

LLM Execution Development.

Concepts:
* Source Code = This (there is not name yet)
* Application = Legacy Source Code
* Compiler = LLM

What would happen if:
* Source code becomes just an opaque resource for being executed.
* If we express the context of the solution (compile the idea).

In classic terms the LLM is the Compiler, Source Code is the Binary, the Programming language is Context Description.

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

# History

After doing a small project in OpenAI i just deleted the chat,
later i decide to add more features but it was not possible
because i did not have the "source code", so some questions hit me:
Why i need to spend time of my life fixing LLM trash?
What if i just compile the idea?
How can i study the idea of others?

So this project is for exploring this questions
