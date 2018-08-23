# encoding=utf8 

from bs4 import BeautifulSoup
import urllib
import nltk
from nltk.corpus import stopwords
 
response = urllib.urlopen('http://php.net/')
html = response.read()
soup = BeautifulSoup(html,"html5lib")
text = soup.get_text(strip=True)
tokens = text.split()
clean_tokens = list()
sr = stopwords.words('english')
for token in tokens:
    if not token in sr:
        clean_tokens.append(token)
freq = nltk.FreqDist(clean_tokens)
for key,val in freq.items():
    print (key.encode("utf8") + ':' + str(val))

freq.plot(20, cumulative=False)
