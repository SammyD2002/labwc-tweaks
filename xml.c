// SPDX-License-Identifier: GPL-2.0-only
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <glib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

static struct ctx {
	char *filename;
	xmlDoc *doc;
	xmlXPathContextPtr xpath_ctx_ptr;
	char *nodename;
	char *value;
	xmlNode *node;
	enum {
		XML_MODE_SETTING = 0,
		XML_MODE_GETTING,
	} mode;
} ctx;

static void
entry(xmlNode *node, char *nodename, char *content)
{
	if (!nodename)
		return;
	if (!strcasecmp(nodename, ctx.nodename)) {
		if (ctx.mode == XML_MODE_SETTING) {
			xmlNodeSetContent(node, (const xmlChar *)ctx.value);
		} else if (ctx.mode == XML_MODE_GETTING) {
			ctx.value = (char *)content;
			ctx.node = node;
		}
	}
}

/*
 * nodename - return simplistic xpath style nodename
 * For example: <A><B><C></C></B></A> is represented by nodename /a/b/c
 */
static char *
nodename(xmlNode *node, char *buf, int len)
{
	if (!node || !node->name) {
		return NULL;
	}

	/* Ignore superflous '/text' in node name */
	if (node->parent && !strcmp((char *)node->name, "text")) {
		node = node->parent;
	}

	buf += len;
	*--buf = 0;
	len--;

	for (;;) {
		const char *name = (char *)node->name;
		int i = strlen(name);
		while (--i >= 0) {
			unsigned char c = name[i];
			*--buf = tolower(c);
			if (!--len)
				return buf;
		}
		node = node->parent;
		if (!node || !node->name) {
			*--buf = '/';
			return buf;
		}
		*--buf = '/';
		if (!--len)
			return buf;
	}
}

static void
process_node(xmlNode *node)
{
	char *content;
	static char buffer[256];
	char *name;

	content = (char *)node->content;
	if (xmlIsBlankNode(node)) {
		return;
	}
	name = nodename(node, buffer, sizeof(buffer));
	entry(node, name, content);
}

static void xml_tree_walk(xmlNode *node);

static void
traverse(xmlNode *n)
{
	process_node(n);
	for (xmlAttr *attr = n->properties; attr; attr = attr->next) {
		xml_tree_walk(attr->children);
	}
	xml_tree_walk(n->children);
}

static void
xml_tree_walk(xmlNode *node)
{
	for (xmlNode *n = node; n && n->name; n = n->next) {
		if (!strcasecmp((char *)n->name, "comment")) {
			continue;
		}
		traverse(n);
	}
}

void
xml_init(const char *filename)
{
	LIBXML_TEST_VERSION

	/*
	 * These two global variables have to be set before xmlReadFile()
	 * in order for xmlSaveFormatFile() to indent properly.
	 */
	xmlKeepBlanksDefault(0);
	xmlIndentTreeOutput = 1;

	ctx.filename = strdup(filename);
	ctx.doc = xmlReadFile(filename, NULL, 0);
	if (!ctx.doc) {
		fprintf(stderr, "warn: xmlReadFile('%s')\n", filename);
	}
	ctx.xpath_ctx_ptr = xmlXPathNewContext(ctx.doc);
	if (!ctx.xpath_ctx_ptr) {
		fprintf(stderr, "warn: xmlXPathNewContext()\n");
		xmlFreeDoc(ctx.doc);
	}
}

void
xml_save(void)
{
	xmlSaveFormatFile(ctx.filename, ctx.doc, 1);
}

void
xml_save_as(const char *filename)
{
	xmlSaveFormatFile(filename, ctx.doc, 1);
}

void
xml_finish(void)
{
	xmlXPathFreeContext(ctx.xpath_ctx_ptr);
	xmlFreeDoc(ctx.doc);
	xmlCleanupParser();
}

void
xml_set(char *nodename, char *value)
{
	ctx.nodename = nodename;
	ctx.value = value;
	ctx.mode = XML_MODE_SETTING;
	xml_tree_walk(xmlDocGetRootElement(ctx.doc));
}

void
xml_set_num(char *nodename, double value)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "%.0f", value);
	ctx.nodename = nodename;
	ctx.value = buf;
	ctx.mode = XML_MODE_SETTING;
	xml_tree_walk(xmlDocGetRootElement(ctx.doc));
}

char *
xml_get(char *nodename)
{
	ctx.nodename = nodename;
	ctx.mode = XML_MODE_GETTING;
	xml_tree_walk(xmlDocGetRootElement(ctx.doc));
	return ctx.value;
}

int
xml_get_int(char *nodename)
{
	ctx.nodename = nodename;
	ctx.mode = XML_MODE_GETTING;
	xml_tree_walk(xmlDocGetRootElement(ctx.doc));
	return ctx.value ? atoi(ctx.value) : 0;
}

int
xml_get_bool_text(char *nodename)
{
	char *value = xml_get(nodename);

	/* handle <foo></foo> and <foo /> where no value has been specified */
	if (!value || !*value) {
		return -1;
	}
	if (!strcasecmp(value, "yes") || !strcasecmp(value, "true")) {
		return 1;
	} else if (!strcasecmp(value, "no") || !strcasecmp(value, "false")) {
		return 0;
	} else {
		return -1;
	}
}

/* case-insensitive */
static xmlNode *
xml_get_node(char *nodename)
{
	ctx.node = NULL;
	ctx.nodename = nodename;
	ctx.mode = XML_MODE_GETTING;
	xml_tree_walk(xmlDocGetRootElement(ctx.doc));
	return ctx.node;
}

char *
xpath_get_content(char *xpath_expr)
{
	xmlChar *ret = NULL;
	xmlXPathObjectPtr object = xmlXPathEvalExpression((xmlChar *)xpath_expr, ctx.xpath_ctx_ptr);
	if (!object) {
		fprintf(stderr, "warn: xmlXPathEvalExpression()\n");
		return NULL;
	}
	if (!object->nodesetval) {
		fprintf(stderr, "warn: no nodesetval\n");
		goto out;
	}
	for (int i = 0; i < object->nodesetval->nodeNr; i++) {
		if (!object->nodesetval->nodeTab[i]) {
			continue;
		}

		/* Just grab the first node and go */
		ret = xmlNodeGetContent(object->nodesetval->nodeTab[i]);
		goto out;

		/*
		 * We could process the node here and do things like:
		 *   xmlNode *children = object->nodesetval->nodeTab[i]->children;
		 *   for (xmlNode *cur = children; cur; cur = cur->next) { }
		 */
	}

out:
	xmlXPathFreeObject(object);
	return (char *)ret;
}

/* case-sensitive */
static xmlNode *
xpath_get_node(xmlChar *expr)
{
	xmlNode *ret = NULL;
	xmlXPathObjectPtr object = xmlXPathEvalExpression(expr, ctx.xpath_ctx_ptr);
	if (!object) {
		fprintf(stderr, "warn: xmlXPathEvalExpression()\n");
		return NULL;
	}
	if (!object->nodesetval) {
		fprintf(stderr, "warn: no nodesetval\n");
		goto out2;
	}

	for (int i = 0; i < object->nodesetval->nodeNr; i++) {
		if (!object->nodesetval->nodeTab[i]) {
			continue;
		}
		ret = object->nodesetval->nodeTab[i];
		break;
	}
out2:
	xmlXPathFreeObject(object);
	return ret;
}

void
xpath_add_node(char *xpath_expr)
{
	if (xml_get_node(xpath_expr)) {
		return;
	}

	/* find existing parent */
	char *parent_expr = strdup(xpath_expr);
	xmlNode *parent_node = NULL;
	while (parent_expr && *parent_expr) {
		parent_node = xpath_get_node((xmlChar *)parent_expr);
		if (parent_node) {
			break;
		}
		char *p = strrchr(parent_expr, '/');
		if (p && *p) {
			*p = '\0';
		} else {
			break;
		}
	}
	assert(parent_expr);
	if (!*parent_expr) {
		/* the whole xpath expression is new, so add to root */
		parent_node = xmlDocGetRootElement(ctx.doc);
	}

	/* add new nodes */
	gchar **nodes = g_strsplit(xpath_expr + strlen(parent_expr), "/", -1);
	for (gchar **s = nodes; *s; s++) {
		if (*s && **s) {
			parent_node = xmlNewChild(parent_node, NULL, (xmlChar *)*s, NULL);
		}
	}
	g_free(parent_expr);
	g_strfreev(nodes);
}
