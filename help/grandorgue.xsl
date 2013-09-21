<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0">
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

</xsl:stylesheet>
