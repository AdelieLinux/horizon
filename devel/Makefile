.POSIX:

all: requirements/html/index.html script/html/index.html vision/html/index.html

requirements/html/index.html: requirements/*.xml master.xsl pretty.css
	@make -C requirements

script/html/index.html: script/*.xml master.xsl pretty.css
	@make -C script

vision/html/index.html: vision/*.xml master.xsl pretty.css
	@make -C vision

upload-all:
	@make -C requirements upload
	@make -C script upload
	@make -C vision upload
