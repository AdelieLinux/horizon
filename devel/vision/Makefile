.POSIX:

all: html/index.html

html/index.html: *.xml ../pretty.css
	@printf 'Vision: Generating HTML...\n'
	@xmlto html -m ../master.xsl -o html vision.xml
	@cp ../pretty.css html/

upload: html/index.html
	@printf 'Uploading Vision...\n'
	@scp html/* awilcox@www.adelielinux.org:/srv/www/adelie/www/horizon-vision/
