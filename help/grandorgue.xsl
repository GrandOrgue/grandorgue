<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:import href="htmlhelp/htmlhelp.xsl"/>
  <xsl:param name="htmlhelp.use.hhk" select="1"/>
  <xsl:param name="htmlhelp.button.next" select="1"/>
  <xsl:param name="htmlhelp.button.previous" select="1"/>
  <xsl:param name="htmlhelp.button.back" select="1"/>
  <xsl:param name="htmlhelp.button.forward" select="1"/>
  <xsl:param name="htmlhelp.hhp" select="'GrandOrgue.hhp'"/>
  <xsl:param name="htmlhelp.hhc" select="'GrandOrgue.hhc'"/>
  <xsl:param name="htmlhelp.hhk" select="'GrandOrgue.hhk'"/>
  <xsl:param name="htmlhelp.chm" select="'GrandOrgue.chm'"/>
  <xsl:param name="htmlhelp.enumerate.images" select="1"/>
  <xsl:param name="htmlhelp.hhc.show.root" select="0"/>
  <xsl:param name="htmlhelp.show.favorites" select="1"/>
  <xsl:param name="htmlhelp.show.advanced.search" select="1"/>
  <xsl:param name="htmlhelp.show.menu" select="1"/>
  <xsl:param name="variablelist.as.table" select="1"/>
  <xsl:param name="make.clean.html" select="1"/>

  <xsl:template name="pi.dbhtml_term-presentation">
    <xsl:param name="node" select="."/>
    <xsl:variable name="presentation">
      <xsl:call-template name="dbhtml-attribute">
        <xsl:with-param name="pis" select="$node/processing-instruction('dbhtml')"/>
        <xsl:with-param name="attribute" select="'term-presentation'"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$presentation = ''">bold</xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$presentation"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="varlistentry" mode="varlist-table">
    <xsl:variable name="presentation">
      <xsl:call-template name="pi.dbhtml_term-presentation">
        <xsl:with-param name="node" select=".."/>
      </xsl:call-template>
    </xsl:variable>
  
    <xsl:variable name="separator">
      <xsl:call-template name="pi.dbhtml_term-separator">
        <xsl:with-param name="node" select=".."/>
      </xsl:call-template>
    </xsl:variable>
    <tr valign="top">
      <xsl:call-template name="tr.attributes">
        <xsl:with-param name="rownum">
          <xsl:number from="variablelist" count="varlistentry"/>
        </xsl:with-param>
      </xsl:call-template>
  
      <td valign="top">
        <p>
        <xsl:call-template name="anchor"/>
        <xsl:choose>
          <xsl:when test="$presentation = 'bold'">
            <strong>
              <xsl:apply-templates select="term"/>
              <xsl:value-of select="$separator"/>
            </strong>
          </xsl:when>
          <xsl:when test="$presentation = 'italic'">
            <em>
              <xsl:apply-templates select="term"/>
              <xsl:value-of select="$separator"/>
            </em>
          </xsl:when>
          <xsl:when test="$presentation = 'bold-italic'">
            <strong>
              <em>
                <xsl:apply-templates select="term"/>
                <xsl:value-of select="$separator"/>
              </em>
            </strong>
          </xsl:when>
          <xsl:otherwise>
            <xsl:apply-templates select="term"/>
            <xsl:value-of select="$separator"/>
          </xsl:otherwise>
        </xsl:choose>
        </p>
      </td>
      <td valign="top">
        <xsl:apply-templates select="listitem"/>
      </td>
    </tr>
  </xsl:template>

</xsl:stylesheet>
