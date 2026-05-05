export SHELL = /bin/bash

.PHONY: clean test

test:
	make -C test

clean:
	@rm -rf build/
