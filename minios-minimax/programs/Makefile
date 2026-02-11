.PHONY: all clean programs generated help

all: programs generated

programs:
	ruby ../tools/build-programs.rb --programs

generated:
	ruby ../tools/build-programs.rb --generated

clean:
	ruby ../tools/build-programs.rb --clean

help:
	@echo "User Programs Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build all programs and generate C arrays"
	@echo "  make programs     - Build only .bin files"
	@echo "  make generated    - Generate only C arrays"
	@echo "  make clean        - Remove all build artifacts"
	@echo ""
	@echo "Note: This Makefile delegates to tools/build-programs.rb"
