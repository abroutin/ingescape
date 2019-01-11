#!/bin/sh
#If you want to publish the site locally, just run the 'hugo' command and open
#the 'public' folder just created.
#If you want to test the site dynamically, run 'hugo server -D' and open http://localhost:1313/
hugo

rsync -avzh -e ssh --progress --delete ./public/ root@ingenuity.io:/home/ingescape/www/ --exclude=/stats
scp htaccess root@ingenuity.io:/home/ingescape/www/.htaccess
ssh root@ingenuity.io 'chown -R ingescape.users /home/ingescape/www'

rsync -avzh -e ssh --progress --delete ./public/ root@94.23.25.84:/home/ingescape/public_html/ --exclude=/stats
scp htaccess root@94.23.25.84:/home/ingescape/public_html/.htaccess
ssh root@94.23.25.84 'chown -R ingescape.ingescape /home/ingescape/public_html'

rm -Rf public

