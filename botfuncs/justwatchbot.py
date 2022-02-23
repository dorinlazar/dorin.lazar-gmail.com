from justwatch import JustWatch
from roboapi import MessageHandler


class JustWatchBot(MessageHandler):
  def shortcode(self) -> str: return 'movie'

  def on_message(self, msg):
    country = 'RO'
    spl = msg.split()
    if len(spl[0]) == 2 and spl[0].upper() == spl[0]:
      country = spl[0]
      msg = ' '.join(spl[1:])
    just_watch = JustWatch(country=country)
    jw_results = just_watch.search_for_item(query=msg)
    if type(jw_results) != dict or 'items' not in jw_results or len(jw_results['items']) == 0:
      return 'N-am găsit niciun film cu numele [' + msg + ']'
    return 'Căutare: ' + msg + '\n' + '\n'.join([self.to_info_display(x, msg) for x in jw_results['items'][:3]])

  def to_info_display(self, info, msg):
    urls = list(set(['<' + u['urls']['standard_web'] + '>' for u in info['offers']])
                ) if 'offers' in info else ['Nu e nicăieri la streaming']
    f = filter(lambda x: x['provider_type'] == 'imdb:score', info['scoring']) if 'scoring' in info else None
    imdb_info = next(f, None) if f is not None else None
    imdb_value = str(imdb_info['value']) if 'value' in imdb_info else ''
    imdb_text = '' if imdb_info is None else 'IMDB: ' + imdb_value
    # next(filter(lambda x: x['provider_type'] == 'imdb:score', info['scoring']))['value']
    title = ''
    if 'title' in info and 'original_release_year' in info:
      title = 'Titlu: [' + info['title'] + '(' + str(info['original_release_year']) + ')]'
    return title + ' ' + imdb_text + '\n' + '\n'.join(urls)
