.POSIX:

all: html/index.html

html/index.html: *.xml ../pretty.css
	@printf 'Requirements Specification: Generating HTML...\n'
	@xmlto html -m ../master.xsl -o html requirements.xml
	@cp ../pretty.css html/

upload: html/index.html
	@printf 'Uploading Requirements Specification...\n'
	@scp html/* awilcox@www.adelielinux.org:/srv/www/adelie/www/horizon-requirements/
