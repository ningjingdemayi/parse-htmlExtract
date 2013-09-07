#ifndef ____PARSE_HTMLEXTRACE_H____
#define ____PARSE_HTMLEXTRACE_H____

#include "ParserSax.h"
#include "tree.h"
#include "HtmlExtract.h"
#include <set>
#include <vector>



namespace HTML
{
	class ParseHtmlExtract : public ParserSax
	{
	public:
		SpiderParseHtml(HtmlExtract & _htmlExtract , std::set<std::string> & _urls , std::string & _source   )
			: htmlExtract(_htmlExtract) , urls(_urls) , source(_source)  
		{
			bodyStart = false;
			scriptStart = false;
			styleStart = false;
			_count = 0;
		}
 
		const tree<Node> &parseTree(const std::string &html);
		const tree<Node> &getTree()
		{ return mHtmlTree; }

	protected:
		virtual void beginParsing();

		virtual void foundTag(Node node, bool isEnd);
		virtual void foundText(Node node);
		virtual void foundComment(Node node);

		virtual void endParsing();

		unsigned int _count ;
		bool is_set;
		bool bodyStart;
		bool scriptStart;
		bool styleStart;
		std::string & source;
		tree<Node> mHtmlTree;
		tree<Node>::iterator mCurrentState;
	    
		HtmlExtract &htmlExtract;
		std::set <std::string> &urls; 

	};
} //namespace Html

#endif
