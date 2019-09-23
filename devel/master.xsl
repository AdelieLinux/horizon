<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:fo="http://www.w3.org/1999/XSL/Format"
                version="1.0">
    <xsl:param name="use.id.as.filename" select="1"/>
    <xsl:param name="admon.graphics" select="1"/>
    <xsl:param name="admon.graphics.path"/>
    <xsl:param name="callout.graphics" select="0"/>
    <xsl:param name="chunk.section.depth" select="0"/>
    <xsl:param name="chunker.output.doctype-public" select="'-//W3C//DTD HTML 4.01 Transitional//EN'"/>
    <xsl:param name="chunker.output.doctype-system" select="'http://www.w3.org/TR/html4/loose.dtd'"/>
    <xsl:param name="chunker.output.encoding" select="'UTF-8'"/>
    <xsl:param name="chunker.output.indent" select="'yes'"/>
    <xsl:param name="html.stylesheet" select="'pretty.css'"/>
    <xsl:param name="page.height" select="'9in'"/>
    <xsl:param name="page.width" select="'6.75in'"/>
    <xsl:param name="page.margin.inner" select="'1.5cm'"/>
    <xsl:param name="page.margin.bottom" select="'9mm'"/>
    <xsl:param name="page.margin.outer" select="'1cm'"/>
    <xsl:param name="page.margin.top" select="'18mm'"/>
    <xsl:template match="section[@role = 'NotInToc']" mode="toc"/>
    <xsl:variable name="adelie.book.title">
        <xsl:call-template name="get.doc.title"/>
    </xsl:variable>
    <xsl:include href="xsl/permalinks.xsl"/>
    <xsl:include href="xsl/section.title.xsl"/>
    <xsl:template name="head.content">
        <xsl:param name="node" select="."/>
        <xsl:param name="title">
            <xsl:value-of select="$adelie.book.title"/>
            <xsl:text> - </xsl:text>
            <xsl:apply-templates select="$node" mode="object.title.markup.textonly"/>
        </xsl:param>

        <xsl:call-template name="user.head.title">
            <xsl:with-param name="title" select="$title"/>
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:if test="$html.base != ''">
            <xsl:call-template name="head.content.base">
                <xsl:with-param name="node" select="$node"/>
            </xsl:call-template>
        </xsl:if>

        <!-- Insert links to CSS files or insert literal style elements -->
        <xsl:call-template name="generate.css"/>

        <xsl:if test="$html.stylesheet != ''">
            <xsl:call-template name="output.html.stylesheets">
                <xsl:with-param name="stylesheets" select="normalize-space($html.stylesheet)"/>
            </xsl:call-template>
        </xsl:if>

        <xsl:if test="$html.script != ''">
            <xsl:call-template name="output.html.scripts">
                <xsl:with-param name="scripts" select="normalize-space($html.script)"/>
            </xsl:call-template>
        </xsl:if>

        <xsl:if test="$link.mailto.url != ''">
            <xsl:call-template name="head.content.link.made">
                <xsl:with-param name="node" select="$node"/>
            </xsl:call-template>
        </xsl:if>

        <xsl:call-template name="head.content.generator">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:if test="$generate.meta.abstract != 0">
            <xsl:call-template name="head.content.abstract">
                <xsl:with-param name="node" select="$node"/>
            </xsl:call-template>
        </xsl:if>

        <xsl:if test="($draft.mode = 'yes' or
                      ($draft.mode = 'maybe' and
                      ancestor-or-self::*[@status][1]/@status = 'draft'))
                      and $draft.watermark.image != ''">
            <xsl:call-template name="head.content.style">
                <xsl:with-param name="node" select="$node"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>
</xsl:stylesheet>
