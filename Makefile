all: deploy

build:
	pxt build

deploy:
	pxt deploy

test:
	jest

test-unit:
	jest

test-shims:
	pxt buildshims

test-pxt:
	pxt test
