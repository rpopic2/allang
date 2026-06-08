SRCS := $(wildcard *.c *.h)
AL   := $(wildcard tests/*.al)
OK   := $(AL:.al=.ok)

CLANGFLAGS :=
ifneq (,$(findstring x86,$(shell uname -m)))
CLANGFLAGS := -masm=intel
endif

.PHONY: all clean
all: $(OK)

alc: $(SRCS)
	./build.sh

tests/%.s: tests/%.al alc
	@./alc $< >/dev/null

tests/%_bin: tests/%.s
	clang $< $(CLANGFLAGS) -o $@

tests/%.ok: tests/%_bin alc
	@$< >/dev/null 2>&1 && touch $@ && echo "PASS $*" || (echo "FAIL $*"; exit 1)

.SECONDARY:

clean:
	rm -f tests/*.s tests/*_bin tests/*.ok
