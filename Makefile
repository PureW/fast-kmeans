


all:
	./waf build_release

debug:
	./waf build_debug

clean:
	./waf clean_release
	./waf clean_debug


