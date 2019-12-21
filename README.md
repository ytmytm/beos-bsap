
# BSAP 0.8
========

(c) 2005 Maciej Witkowiak <ytm@elysium.pl>
http://members.elysium.pl/ytm/html/beos.html
http://ytm.bossstation.dnsalias.org/html/beos.html

BSAP is an English-Polish, Polish-English dictionary program that uses dictionary data files
from sap. It can also read dictionary data from Collins dictionary released by YDP
http://www.ydp.com.pl
I have used ideas from sap and kydpdict code thus BSAP is under GNU/GPL terms.

## RELEASE
The zip file contains last public release. It is included here to preserve BeOS file attributes.

## Screenshots

![Screenshot](/images/bsap1.png?raw=true "BSAP")

![Screenshot](/images/bsap2-clip.png?raw=true "Example of clipboard tracking")


## REQUIREMENTS
The program requires BeOS to be configured for Polish keymap and installed fonts with Polish
letters.
The program also requires SpLocale library. http://bebits.com/app/1869
For SQL data management, you need Sqlite2 http://bebits.com/app/3156
To use YDP dictionary you have to copy data files from already installed Windows program.
The best location for them would be the directory with SAP and SQL files. You need these files:
dict100.dat, dict101.dat, dict100.idx, dict101.idx. (everything lowercase).

## CONFIGURATION
Upon the first execution the program will ask for directory that contains dictionary files -
dvp_1.dic and dvp_2.dic. These are SAP dictionary files. SQL database (SAP converted into
sqlite2 database) - bsapdict.sq2 should be in the very same work directory.
Of course YDP data files might be in another location.
This is important - you need to select a directory, not files within it.
The program keeps its configuration file in /home/config/settings/bsap. It is a plain text file
that you can edit or, in case of problems, delete.

## USAGE
Trivial :). However there are some things worth to be mentioned:
The program has two search modes - plain search and fuzzy search.
In plain search mode as the search result there is returned that word which shares the most
common characters (from the left) with given query.
Fuzzy search works by finding words that are similar to the given one. It utilizes edit distance
algorithm for that purpose. Due to performance reasons with fuzzy search caseness does
matter. You can control how similar the words should be by fuzzy factor menu option.

## CLIPBOARD
If "Clipboard tracking" option is on, then any text copied to the clipboard will be automatically
intercepted and searched in the dictionary. If combined with "Popup window" option - the
dictionary window will place itself above all other windows in the workspace.

## KEYBOARD SHORTCUTS
There is only one invisible shortcut - ESC key will clear the edit line and move focus there.
Otherwise the program should be quite comfortable controlled with keyboard only.

## SQL
SQL engine (SQ2 - SqLite2) uses only bsapdict.sq2 data file, that you can freely modify with
sqlite2 program and SQL language. Whole database schema consists of these two lines:
```
CREATE TABLE dictionaries (id INTEGER, name TEXT, desc TEXT, PRIMARY KEY(id));
CREATE TABLE words (id INTEGER, dictionary INTEGER, key TEXT, desc TEXT, PRIMARY KEY(id,dictionary,key));
```
The 'dictionaries' table keeps track of all dictionaries available in database and "SQL data source" menu option uses it to allow you to choose two active dictionaries.
The 'words' table has words definitions. For a given 'dictionary' value the pair id-key must be unique. The definitions must be encoded in UTF8.
Program assumes that the dictionary with id==0 is always available. This is default and fallback
if there is no configuration file.
If BSAP complains about no results from database it means that there are no words in
dictionary with given id. Just edit or delete configuration file in /boot/home/config/settings/bsap
then.
With large database file reading its index may take some time so on the webpage of BSAP
I have put some source files that you can use with sqlite program and build your own
bsapdict.sq2 from scratch with given set of dictionaries.

## KNOWN BUGS
I guess that my default colour scheme is ugly and can be only changed :). I don't like some
option names also.

## FINAL WORD
If you're a programmer take a look at the end of INSTALL.txt file for some info about code that
might be useful in your projects.
