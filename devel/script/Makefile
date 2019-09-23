.POSIX:

all: html/index.html

html/index.html: *.xml ../pretty.css
	@printf 'HorizonScript Specification: Generating HTML...\n'
	@xmlto html -m ../master.xsl -o html script.xml
	@cp ../pretty.css html/

upload: html/index.html
	@printf 'Uploading HorizonScript Specification...\n'
	@scp html/* awilcox@www.adelielinux.org:/srv/www/adelie/www/horizon-script/
