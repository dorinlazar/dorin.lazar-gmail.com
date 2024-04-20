#include "article.hpp"
#include "feed_digest.hpp"
#include "expat_parser.hpp"

#include <memory>
#include <vector>

enum class FeedSystemType { Atom, Rss };
struct FeedSystem {
  const FeedSystemType type;
  const std::string channel_tag_name;
  const std::string item_name;
  const std::string last_build_date_name;
  const std::string guid_name;
  const std::string publish_item_date_name;
};

FeedSystem AtomSystem = {FeedSystemType::Atom, "feed", "entry", "updated", "id", "published"};
FeedSystem RssSystem = {FeedSystemType::Rss, "channel", "item", "lastBuildDate", "guid", "pubDate"};

std::string GetOrDefault(const std::map<std::string, std::string>& attributes, const std::string& key,
                         const std::string& default_value = "") {
  auto it = attributes.find(key);
  if (it == attributes.end()) {
    return default_value;
  }
  return it->second;
}

class FeedParser : public XmlParser {
public:
  FeedParser() {}
  ~FeedParser() override = default;

  void StartElement(const std::string& name, const std::map<std::string, std::string>& attributes) override {
    if (!m_feed_system) {
      if (name == "rss") {
        m_feed_system = std::make_unique<FeedSystem>(RssSystem);
      } else if (name == "feed") {
        m_feed_system = std::make_unique<FeedSystem>(AtomSystem);
      }
      return;
    }
    if (!m_in_channel) {
      m_in_channel = name == m_feed_system->channel_tag_name;
      return;
    }
    if (m_in_item) {
      if (name == "link" && GetOrDefault(attributes, "rel", "alternate") == "alternate") {
        m_current_article.link = GetOrDefault(attributes, "href");
      }
      if (name == "enclosure" && m_current_article.link.empty()) {
        m_current_article.link = GetOrDefault(attributes, "url");
      }
    } else {
      m_in_item = name == m_feed_system->item_name;
      if (m_in_item) {
        m_current_article = Article();
      }
    }

    m_current_data = "";
  }

  void EndElement(const std::string& name) override {}
  void CharacterData(const std::string& data) override {}

  // void endElement(const XML_Char* name) override {
  //   if (name == std::string("item")) {
  //     articles.push_back(currentArticle);
  //   }
  //   currentElement = "";
  // }

  // void charData(const XML_Char* s, int len) override {
  //   std::string data(s, len);
  //   if (currentElement == "title") {
  //     currentArticle.title += data;
  //   } else if (currentElement == "link") {
  //     currentArticle.link += data;
  //   } else if (currentElement == "description") {
  //     currentArticle.description += data;
  //   }
  // }

private:
  std::unique_ptr<FeedSystem> m_feed_system;
  bool m_in_channel{false};
  bool m_in_item{false};
  std::string m_current_data;
  std::vector<Article> m_articles;
  Article m_current_article;
};
