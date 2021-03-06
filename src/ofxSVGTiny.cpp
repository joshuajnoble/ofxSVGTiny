#include "ofxSVGTiny.h"

ofxSVGTiny::~ofxSVGTiny()
{
	paths.clear();
}

void ofxSVGTiny::load(string path)
{
	path = ofToDataPath(path);
	
	ofBuffer buffer = ofBufferFromFile(path);
	size_t size = buffer.size();
	
	struct svgtiny_diagram *diagram = svgtiny_create();
	svgtiny_code code = svgtiny_parse(diagram, buffer.getText().c_str(), size, path.c_str(), 0, 0);
	
	if (code != svgtiny_OK)
	{
		fprintf(stderr, "svgtiny_parse failed: ");
		switch (code)
		{
			case svgtiny_OUT_OF_MEMORY:
				fprintf(stderr, "svgtiny_OUT_OF_MEMORY");
				break;
			case svgtiny_LIBXML_ERROR:
				fprintf(stderr, "svgtiny_LIBXML_ERROR");
				break;
			case svgtiny_NOT_SVG:
				fprintf(stderr, "svgtiny_NOT_SVG");
				break;
			case svgtiny_SVG_ERROR:
				fprintf(stderr, "svgtiny_SVG_ERROR: line %i: %s",
						diagram->error_line,
						diagram->error_message);
				break;
			default:
				fprintf(stderr, "unknown svgtiny_code %i", code);
				break;
		}
		fprintf(stderr, "\n");
	}
	
	setupDiagram(diagram);
	
	svgtiny_free(diagram);
}

void ofxSVGTiny::draw()
{
	for (int i = 0; i < paths.size(); i++)
	{
		paths[i]->draw();
	}
}


void ofxSVGTiny::setupDiagram(struct svgtiny_diagram *diagram)
{
	for (int i = 0; i < diagram->shape_count; i++)
	{
		if (diagram->shape[i].path)
		{
			setupShape(&diagram->shape[i]);
		}
		else if (diagram->shape[i].text)
		{
			printf("text: not implemented yet\n");
		}
	}
}

void ofxSVGTiny::setupShape(struct svgtiny_shape *shape)
{
	float *p = shape->path;
	
	ofPath *path = new ofPath();
	paths.push_back(ofPathRef(path));
	
	path->setFilled(false);
	
	if (shape->fill != svgtiny_TRANSPARENT)
	{
		path->setFilled(true);
		path->setFillHexColor(shape->fill);
	}
	
	if (shape->stroke != svgtiny_TRANSPARENT)
	{
		path->setStrokeWidth(shape->stroke_width);
		path->setStrokeHexColor(shape->stroke);
	}
	
	for (int i = 0; i < shape->path_length;)
	{
		if (p[i] == svgtiny_PATH_MOVE)
		{
			path->moveTo(p[i + 1], p[i + 2]);
			i += 3;
		}
		else if (p[i] == svgtiny_PATH_CLOSE)
		{
			path->close();
			
			i += 1;
		}
		else if (p[i] == svgtiny_PATH_LINE)
		{
			path->lineTo(p[i + 1], p[i + 2]);
			i += 3;
		}
		else if (p[i] == svgtiny_PATH_BEZIER)
		{
			path->bezierTo(p[i + 1], p[i + 2],
						   p[i + 3], p[i + 4],
						   p[i + 5], p[i + 6]);
			i += 7;
		}
		else
		{
			printf("error\n");
			i += 1;
		}
	}
}
