#include "ParseHtmlExtract.h"
#include "wincstring.h"

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <time.h>


//#define DEBUG
#include "debug.h"

#define TAG_NAME_MAX 10

using namespace std;
using namespace htmlcxx; 
using namespace HTML; 
using namespace kp; 




const tree<Node> & Html::ParseHtmlExtract::parseTree( const std::string &html )
{
	this->parse(html);
	return this->getTree();
}

void Html::ParseHtmlExtract::beginParsing()
{
	mHtmlTree.clear();
	tree<HTML::Node>::iterator top = mHtmlTree.begin();
	HTML::Node lambda_node;
	lambda_node.offset(0);
	lambda_node.length(0);
	lambda_node.isTag(true);
	lambda_node.isComment(false);
	mCurrentState = mHtmlTree.insert(top,lambda_node);
}

void Html::ParseHtmlExtract::foundTag( Node node, bool isEnd )
{

	// 如果不是如下的几个，则返回
	if(
		(strcasecmp(node.tagName().c_str(), "a")) &&
		(strcasecmp(node.tagName().c_str(), "meta")) &&
		(strcasecmp(node.tagName().c_str(), "body")) &&
		(strcasecmp(node.tagName().c_str(), "title")) && 
		(strcasecmp(node.tagName().c_str(), "script")) &&
		(strcasecmp(node.tagName().c_str(), "style"))
		)
		return ;

	if(strcasecmp(node.tagName().c_str(), "body") == 0)
	{
		bodyStart = true;
	}

	if(strcasecmp(node.tagName().c_str(), "style") == 0)
	{
		styleStart = true;
	}

	if(strcasecmp(node.tagName().c_str(), "script") == 0)
	{
		scriptStart = true;
	}

	if(strcasecmp(node.tagName().c_str(), "meta") == 0)
	{
		node.parseAttributes();
		if(strcasecmp(node.attribute("name").second.c_str(), "description") == 0)
		{
			htmlExtract.description = node.attribute("content").second;
			//cout << node.attribute("name").second << ":" <<  node.attribute("content").second << endl << endl;
		}
		if(strcasecmp(node.attribute("name").second.c_str(), "keywords") == 0)
		{
			htmlExtract.keywords = node.attribute("content").second;
			//cout << node.attribute("name").second<< ":" << node.attribute("content").second << endl << endl;
		}

		return;
	}

	if (!isEnd) 
	{
		//append to current tree node
		tree<HTML::Node>::iterator next_state;
		next_state = mHtmlTree.append_child(mCurrentState, node);
		mCurrentState = next_state;
	} 
	else 
	{
		//Look if there is a pending open tag with that same name upwards
		//If mCurrentState tag isn't matching tag, maybe a some of its parents
		// matches
		vector< tree<HTML::Node>::iterator > path;
		tree<HTML::Node>::iterator i = mCurrentState;
		bool found_open = false;
		while (i != mHtmlTree.begin())
		{
#ifdef DEBUG
			cerr << "comparing " << node.tagName() << " with " << i->tagName()<<endl<<":";
			if (!i->tagName().length()) cerr << "Tag with no name at" << i->offset()<<";"<<i->offset()+i->length();
#endif
			assert(i->isTag());
			assert(i->tagName().length());

			bool equal;
			const char *open = i->tagName().c_str();
			const char *close = node.tagName().c_str();
			equal = !(strcasecmp(open,close));


			if (equal) 
			{

				DEBUGP("Found matching tag %s\n", i->tagName().c_str());
				//Closing tag closes this tag
				//Set length to full range between the opening tag and
				//closing tag
				i->length(node.offset() + node.length() - i->offset());
				i->closingText(node.text());

				mCurrentState = mHtmlTree.parent(i);
				found_open = true;

				Node &_node = *i;


				


				if(strcasecmp(_node.tagName().c_str(), "title") == 0)
				{
					std::string _title = source.substr(_node.offset() + 7 , _node.length() - 15);
					htmlExtract.title = _title;
					//cout << "title:" << _title << endl << endl;
				}

				if(strcasecmp(_node.tagName().c_str(), "body") == 0)
				{
					bodyStart = false;
				}

				if(strcasecmp(_node.tagName().c_str(), "script") == 0)
				{
					scriptStart = false;
				}

				if(strcasecmp(_node.tagName().c_str(), "style") == 0)
				{
					styleStart = false;
				}

				if(strcasecmp(_node.tagName().c_str(), "a") == 0)
				{


					/*try
					{*/
						//static const boost::regex regTemp("^http(s?):\/\/(?:[A-za-z0-9-]+\.)+[A-za-z]{2,4}$");

						//ofstream _of("urls2" , ios::app);
						_node.parseAttributes();
						string _url = _node.attribute("href").second;
						//_of << _url <<  endl;
						if(_url.size() <= 10)
							break;
						string temp = _url.substr(0 , 8);
						string temp1 = _url.substr( 0 , 7);
						if(!strcasecmp(_url.substr(0 , 8).c_str(), "https://"))
						{
							_url = _url.substr(8 , _url.length() - 8);
							unsigned int itemp1 = _url.find_first_of('?');
							unsigned int itemp2 = _url.find_first_of('/');
							_url = itemp1 >= itemp2 ?_url.substr(0 , itemp2) :_url.substr(0 , itemp1);
							_url = "https://" + _url;

							/*if(_url.size() <= 280)
								if(boost::regex_match( _url, regTemp))*/
									urls.insert(_url);
							//urls.push_back(_url);
							//_of << _url << "\n";
						}
						if(!strcasecmp(_url.substr(0 , 7).c_str(), "http://"))
						{
							_url = _url.substr(7 , _url.length() - 7 );
							unsigned int itemp1 = _url.find_first_of('?');
							unsigned int itemp2 = _url.find_first_of('/');
							_url = itemp1 >= itemp2 ?_url.substr(0 , itemp2) :_url.substr(0 , itemp1);
							//if(urls.size() == 595)
							//{
							//	cout << _url << endl;
							//}
							_url = "http://" + _url;

							/*if(_url.size() <= 280)
								if(boost::regex_match( _url, regTemp))*/
									urls.insert(_url );
							//urls.push_back(_url);
							//_of << _url << "\n";
						}
						/*}
						catch(boost::regex::bad_expression  ec )
						{
						writeToLog("解析错误" , rtc);
						writeToLog(ec.message() , rtc);
						break;
						}*/

					//_of.close();
					break;
				}

				break;
			} 
			else 
			{
				path.push_back(i);
			}

			i = mHtmlTree.parent(i);
		}

		if (found_open)
		{
			//If match was upper in the tree, so we need to invalidate child
			//nodes that were waiting for a close
			for (unsigned int j = 0; j < path.size(); ++j)
			{
				//				path[j]->length(node.offset() - path[j]->offset());
				mHtmlTree.flatten(path[j]);
			}
		} 
		else 
		{
			DEBUGP("Unmatched tag %s\n", node.text().c_str());

			// Treat as comment
			node.isTag(false);
			node.isComment(true);
			mHtmlTree.append_child(mCurrentState, node);
		}
	}
	
}

int trim(const char * des , string &ret , int size)
{
	int temp1 = size;
	int tempUseless = 0;
	while(*des != '\0' && size > 0)
	{
		if(*des != '\t' && *des != ' ' && *des != '\r' && *des != '\n')
		{
			ret.push_back(*des);
		}
		else
			++ tempUseless;
		++ des;
		--size;
	}
	return temp1 - tempUseless;
}

void Html::ParseHtmlExtract::foundText( Node node )
{
	unsigned int &size = _count;
	if(bodyStart)
	{
		if(!( scriptStart || styleStart ) )
		{
			string temp = node.text();
			if(temp.size() <= 0)
				return;
			temp = temp.substr(0 , temp.find_first_of('&'));
			string ret;
			trim(temp.c_str() , ret , temp.size());
			size += ret.size();
			if(ret.size() != 0)
			{
				ret.push_back(' ');
				htmlExtract.full_txt += ret;
			}
			//cout << ret << endl;
			//htmlExtract.full_txt += temp;


			/*ofstream _of ("fulltext" , ios::app);
			_of << ret.size() << "++++++++++" << ret << endl;
			_of.close();*/
		}
	}
	if(size >= 200)
		bodyStart = false;
	

	//Add child content node, but do not update current state
	//mHtmlTree.append_child(mCurrentState, node);
}


void Html::ParseHtmlExtract::foundComment( Node node )
{
	//Add child content node, but do not update current state
	//mHtmlTree.append_child(mCurrentState, node);
	return;
}

void Html::ParseHtmlExtract::endParsing()
{
	/*tree<HTML::Node>::iterator top = mHtmlTree.begin();
	top->length(mCurrentOffset);*/
	mHtmlTree.clear();
}
