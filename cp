#!/bin/bash
############################################
#                                          #
#   Scirpt That Automate The Installation  #
#       Of ClassicPress and WordPress      #
#         Using Nginx Web Server           #
#                                          #
#   Author: The_Devsrealm_Guy              #
#   Website: https://devsrealm.com         #
#   Last Edited: October 2020              #
#                                          #
############################################

#
#   
#   Credit Goes To Wade Striebel, Without him, I Won't Have Gotten To 
#   This stage,  He is a Big Inspiration To Me, He is The Only Guy That Won't
#   Get Tired of Me Sending numerous of Messages Even at Late Hour, That Dude is a God Sent.
#
#   It is Hard to use words to convey my heartfelt appreciation for the kind of help you rendered me. 
#   You are indeed a good friend. I am very grateful.
#
#   Back to Program :)
#

#
# Check if user have the proper have privileges
#

if [ $(id -u) -ne 0 ]
  then
	echo -e "\tPlease Run This Program as a Root user only" >&2
	exit 1
fi

#######################################
#   WE CREATE ALL THE FUNCTIONS HERE  #
#######################################

#
#   errorchecker()
#
#   Check Error at certain place and exit if it not equal to zero
#

#
# Include The Main Backup Function
#
source mainbackup

errorchecker() {

    errorstat=$1

    if [[ $errorstat != 0 ]];then
    echo "Installation Error, Check $logfile"
    exit 1
    fi

}

errorchecker_certbot() {

    errorstat=$1

    if [[ $errorstat != 0 ]];then
    echo "Let's Encrypt Failed, Check $logfile"
    exit 1
    fi

}

errorchecker_restore() {

    errorstat=$1
    
    if [[ $errorstat != 0 ]];then
    echo "Restoration Error, Check $logfile"
    exit 1
    fi

}



#
#   pause()
#
#   Ask the user to press ENTER and wait for them to do so
#
pause()
{
    echo
    echo -e "\t\t\t\tHit <ENTER> to continue: \c"
    read trash
}

pause_webserver()
{
    clear
    echo
    echo -e "\t\t\t\t$1 $2 Installed" "\xE2\x9C\x94"  "\n\\n\t\t\t\tHit <ENTER> to continue: \n"
    read trash

}
#
#   yes_no()
#
#   A function to display a string (passed in as $*), followed by a "(Y/N)?",
#   and then ask the user for either a Yes or No answer.  Nothing else is
#   acceptable.
#   If Yes is answered, yes_no() returns with an exit code of 0 (True).
#   If No is answered, yes_no() returns with an exit code of 1 (False).
#
yes_no()
{
    #
    #   Loop until a valid response is entered
    #
    while :
    do
        #
        #   Display the string passed in $1, followed by "(Y/N)?"
        #   The \c causes suppression of echo's newline
        #
        echo -e "\t\t\t\t$* (Y/N)? \c"

        #
        #   Read the answer - only the first word of the answer will
        #   be stored in "yn".  The rest will be discarded
        #   (courtesy of "junk")
        #
        read yn junk

        case $yn in
            y|Y|yes|Yes|YES)
                return 0;;        # return TRUE
            n|N|no|No|NO)
                return 1;;        # return FALSE
            *)
                echo -e "\t\t\t\tCould You Please answer Yes or No."
                ;;
                #
                # and continue around the loop ....
                #
        esac
    done
}

#
#   A spinner while long process is running
# 

 spinner() {
    local pid=$!
    local delay=1
    local spinstr='|/-\'
    while [ "$(ps a | awk '{print $1}' | grep $pid)" ]; do
        local temp=${spinstr#?}
        printf " [%c]  " "$spinstr"
        local spinstr=$temp${spinstr%"$temp"}
        sleep $delay
        printf "\b\b\b\b\b\b"
    done
    printf "    \b\b\b\b"
}

progress_bar()
{
    for ((k = 0; k <= 10 ; k++))
    do
        echo -e -n "\t\t\t\t[ "
        for ((i = 0 ; i <= k; i++)); do echo -n "###"; done
        for ((j = i ; j <= 10 ; j++)); do echo -n "   "; done
        v=$((k * 10))
        echo -n " ] "
        echo -n "$v %" $'\r'
        sleep 0.7
    done
    echo
}


#
#   web_server function
#
#   A function check if the web server is installed, if no,
#   we follow by a "(Y/N)?" to install it
# 


web_server()
{
 if command -v nginx 2>> "${logfile}" >/dev/null
    then
      echo
      echo -e "\t\t\t\tnginx is available\n"

      else
        #
        #   Ask if it should be created
        #
        echo
        echo -e "\t\t\t\tNginx Seems To Be Missing\n"
          if yes_no "Install Nginx Web Server"
          then
          echo -e "\t\t\t\tInstalling Nginx From The Official Nginx Repo"
          sudo wget https://nginx.org/keys/nginx_signing.key 2>> "${logfile}" >/dev/null &
          sudo apt-key add nginx_signing.key 2>> "${logfile}" >/dev/null &

          #   We add the below lines to sources.list to name the repositories 
          #   from which the NGINX Open Source source can be obtained:
          #   The lsb_release automatically adds the distro codename
          echo "deb [arch=amd64] http://nginx.org/packages/mainline/ubuntu $(lsb_release -cs) nginx" \
              | sudo tee -a /etc/apt/sources.list >/dev/null &

          echo "deb-src http://nginx.org/packages/mainline/ubuntu $(lsb_release -cs) nginx" \
              | sudo tee -a /etc/apt/sources.list >/dev/null &


          sudo apt-get update 2>> "${logfile}" >/dev/null &
          sudo apt-get -y install nginx 2>> "${logfile}" >/dev/null &

          # Spinning, While the program installs
          spinner  

          #   Recheck if nginx is installed
          #   Pause to give the user a chance to see what's on the screen
          #
          if command -v nginx 2>> "${logfile}" >/dev/null
            then
            pause_webserver Nginx
            else
            echo -e "\t\t\t\tCouldn't Install Nginx"
            return 1
          fi

          else

        #
        #   They didn't want to Install Nginx 
        #

          return 1

          fi

        return 0
fi
}

#
#   install_certbot()
#
#   The certbot automation function
#

install_certbot()
{

    echo -e "\t\t\t\tYour Email Address: \c"
    read email
    certbot --nginx -d "$websitename" -d "www.$websitename" -m "$email" --agree-tos --redirect --hsts --staple-ocsp 2>> "${logfile}" >/dev/null &
    errorchecker_certbot $?
    echo
    echo -e "\t\t\t\tDone\n"
    systemctl restart nginx

}

#
#   website_secure()
#
#   Secure website using Let’s Encrypt SSL
#

      
website_secure()
{
  if command -v certbot 2>> "${logfile}" >/dev/null
    then
      echo
      echo -e "\t\t\t\tcertbot is available\n" "\xE2\x9C\x94\n"

      if yes_no "Do you want to secure another website"
       then

       read -p  $'\t\t\t\t'"The Name of New Website You want to secure: " websitename
        #
        # Call The install_certbot function
        #
        install_certbot
        fi
   # read trash

   else
        #
        #   Ask if it should be Installed
        #
        echo
        echo -e "\t\t\t\tCertbot Seems To Be Missing\n"
        if yes_no "Install Certbot"
        then
        sudo apt-get update 2>> "${logfile}" >/dev/null &
        sudo apt-get -y install python-certbot-nginx 2>> "${logfile}" >/dev/null &
        # Spinning, While the program installs
        spinner

        #
        #   If we got here, then it means that we are done with installing certbot
        #   Call The install_certbot function away
        #
        install_certbot

       # reload nginx
        sudo systemctl enable nginx 2>> "${logfile}" >/dev/null &
        sudo systemctl reload nginx 2>> "${logfile}" >/dev/null &
        #
        #   Pause to give the user a chance to see what's on the screen
        #

        else
            echo
            echo -e "\t\t\t\tCouldn't Install certbot, check error log"
            return 1

        fi

  return 0
  fi
}

#
#   Usage Message FUNCTION
#
#   ************************************************************
#   ***Warning*** This comment would be long, it's for reference:
#   *************************************************************
#   The Usage function I created here is a bit tricky, this is what happens,
#   I take $1 which is supposed to represent the script name, and it then stores it in a little variable called script. 
#
#   Shouldn't $1 be a first positional argument, it shouldn't be a script name right? That is correct, but consider we call the 
#   usage function as so:
#
#   usage $0 websitename 
#
#   The usage word above isn't a parameter, it is a function, and it takes the $0 (script name) as its first positional argument 
#   ($1), and it then stores it in the variable script in the usage function. 
#    
#    The shift in the function skips the first parameter, this way we can separate the script name from the rest of the   
#     paramater, so, whatever other parameter you pass e.g if you
#    pass $2 it would now represent $1, and if you pass $3, it would now represent $2 and so on. $* in the function makes us
#     perform test on all the other arguments, even if you supply 20.

#   "basename" is used to transform "/home/devsrealm/install_classicpress"
#   into "install_classicpress"
usage()
{
    script=$1
    shift
    echo
    echo -e "\t\t\t\tUsage: $(basename "$script") $*\n" 1>&2

    exit 2
}


#   quit Function
#
#   This Prompt the user to exit the program, if they choose to, 
#   an exit code is provided in the first argument ($1)
#
quit()
{
    #
    #   Store the exit code away, coz calling another function
    #   overwrites $1.
    #
    code=$1

    if yes_no "Do you really wish to exit"
    then
        exit "$code"           #  exit using the supplied code.
    fi
}

#
#   website_create()
#
#   Create records for our website
#
website_create() 
{
    # Check if the nginx site-available and site-enabled is created, if no create it
    site_available="/etc/nginx/sites-available"
    site_enabled="/etc/nginx/sites-enabled"
    [ -d $site_available ] || sudo mkdir -p $site_available 
    [ -d $site_enabled ] || sudo mkdir -p $site_enabled

    # 
    #   mktemp will create the file or exit with a non-zero exit status, 
    #   this way, you can ensures that the script will exit if mktemp is unable to create the file.
    #
    #   Note: The inclusion of /etc/nginx/sites-enabled/*; is no longer need, I'll leave this for reference
    #
    #   cat ngx_conf_with_caching | sed '/conf.d/a  \\tinclude /etc/nginx/sites-enabled/*;' | awk '! (/sites-enabled/ && seen[$0]++)' > $TMPFILE
    #
    #
    #   This adds the include /etc/nginx/sites-enabled/*; in the nginx config file if it isn't alread there
    #   We also removed any duplicate of /etc/nginx/sites-enabled/*
    #
    #   Note: I added two tabs to make the format of the nginx config consistent
    #    \t is one tab \\t is two tab, if you want three tab, you do \\t\t, yh, sed is crazy
    #

     TMPFILE=$(mktemp /tmp/nginx.conf.XXXXXXXXXX) || exit 1
     cat ngx_conf_with_caching > "$TMPFILE"
     sudo cp -f "$TMPFILE" /etc/nginx/nginx.conf # move the temp to nginx.conf
     # remove the tempfile
     rm "$TMPFILE"

    #
    # Get Server IP address that is used to reach the internet
    # We ge the source Ip, we then use sed to match the string source /src/ 
    # s/             # begin a substitution (match)
    # .*src *      # match anything leading up to and including src and any number of spaces
    #  \([^ ]*\)    # define a group containing any number of non spaces
    #  .*           # match any trailing characters (which will begin with a space because of the previous rule).
    #     /              # begin the substitution replacement
    #  \1           # reference the content in the first defined group
    #
    # Note: This is not useful for now
    ip="$(ip route get 8.8.8.8 | sed -n '/src/{s/.*src *\([^ ]*\).*/\1/p;q}')"

    #
    #   Remove both occurrences of default_server, servername and point
    #   the root directory to the new website root for your newly copied config.
    # 
    #   mktemp will create the file or exit with a non-zero exit status, 
    #   this way, you can ensures that the script will exit if mktemp is unable to create the file.
    #



    TMPFILE=$(mktemp /tmp/default.nginx.XXXXXXXX) || exit 1

    #
    #   This code was originally:
    # cat ngx_serverblock | sudo sed -e "s/domain.tld/$websitename www.$websitename/g" -e "s/\/var\/www\/wordpress/\/var\/www\/$websitename/" > "$TMPFILE"
    #   Which is wrong and known as the useless use of cat, It's more efficient and less roundabout to simply use redirection.
    #
    #   So, what I did here was first redirecting the content of < "ngx_serverblock" to sed program, I then redirect the output 
    #   of whatever I get to the > TMPFILE
    #

    sed -e "s/domain.tld/$websitename www.$websitename/g" -e "s/\/var\/www\/wordpress/\/var\/www\/$websitename/" < ngx_serverblock > "$TMPFILE"

    sudo cp -f "$TMPFILE" $site_available/"$websitename"

    # remove the tempfile
    rm "$TMPFILE"
    #

    #
    #   Create a directory for the root directory if it doesn't already exist
    #

    if [ ! -d /var/www/"$websitename" ];then
    sudo mkdir -p /var/www/"$websitename"
    fi

    #   
    #   Nginx comes with a default server block enabled (virtual host), let’s remove the symlink, we then add the new one
    #

    if [ -f $site_enabled/default ];then

      sudo unlink $site_enabled/default 2>> "${logfile}" >/dev/null &

    fi


    errorchecker $?

    #   
    #   Check if symbolik link exist already
    #


    if [ ! -f $site_enabled/"$websitename" ];then

          sudo ln -s $site_available/"$websitename" /etc/nginx/sites-enabled/ 2>> "${logfile}" >/dev/null &

    fi

    # reload nginx
    sudo systemctl start nginx 2>> "${logfile}" >/dev/null &
    sudo systemctl enable nginx  2>> "${logfile}" >/dev/null &
    sudo systemctl reload nginx 2>> "${logfile}" >/dev/null &

    errorchecker $?
    #
    #   Install and Configure Mariadb and PHP
    #`
    install_mariadb_php
}

#
#   This Automate mysql secure installation for debian-baed systems
# 
#  - You can set a password for root accounts.
#  - You can remove root accounts that are accessible from outside the local host.
#  - You can remove anonymous-user accounts.
#  - You can remove the test database (which by default can be accessed by all users, even anonymous users), 
#    and privileges that permit anyone to access databases with names that start with test_. 
#
#    Tested on Ubuntu 18.04
#

mysql_secure_installation() {
    echo
    while : # Unless Password Matches, Keep Looping
    do

        echo -e "\t\t\t\tSetup mysql root password: \c"
        read -s mysqlpass

        echo -e "\t\t\t\tEnter Password Again: \c"
        read -s mysqlpass2

        
        # 
        #   Checking if both passwords match
        #

        if [ "$mysqlpass" != "$mysqlpass2" ]; then
            echo
            echo -e "\t\t\t\tPasswords do not match, Please Try again"
        else
            echo
            echo -e "\t\t\t\tPasswords Matches, Moving On..." 
            echo
            break
        fi

    done # Endwhile loop

    TMPFILE=$(mktemp /tmp/mysql_secure_installation.XXXXXXXXXX) || exit 1
    #
    #   This code was originally cat mysql_secure_installation.sql | sed -e "s/123456789/$mysqlpass/" > "$TMPFILE"
    #   Which is wrong and known as the useless use of cat, It's more efficient and less roundabout to simply use redirection.
    #
    #   So, what I did here was first redirecting the content of < "mysql_secure_installation.sql" to sed program, I then redirect the output 
    #   of whatever I get to the > TMPFILE
    #
    sed -e "s/123456789/$mysqlpass/" < mysql_secure_installation.sql > "$TMPFILE"
    sudo cp -f "$TMPFILE" mysql_secure_installation.sql # move the temp to mysql_secure_installation.sql
    # remove the tempfile
    rm "$TMPFILE"

    #
    #   The s silences errors and the f forces the commands to continue even if one chokes.
    #   The u relates to the username that immediately follows it which—in this case—is clearly root.
    #
    mysql -sfu root < "mysql_secure_installation.sql"


}


#
#   install_mariadb_php()
#
#   This Downloads and Configure WordPress or ClassicPress
#
install_mariadb_php()
{
  
  #
  # Let's Install PHP and Mariadb
  #
  
if command -v mariadb 2>> "${logfile}" >/dev/null && command -v php 2>> "${logfile}" >/dev/null
    then
      echo
      echo -e "\t\t\t\tMariadb and PHP is available\n"

      if yes_no "Do you want to skip into setting up CP or WP"
      then
        #
        # Call The install_cp_wp function
        #
        install_cp_wp

        fi

        #
        #   Ask if it should be created
        #
     else
        echo
        echo -e "\t\t\t\tPHP or mariadb Seems To Be Missing\n"
          if yes_no "Install PHP and mariadb"
          then

          sudo apt-get -y install mariadb-server php7.2 php7.2-cli php7.2-fpm php7.2-mysql \
          php7.2-json php7.2-opcache php7.2-mbstring php7.2-xml php7.2-gd \
          php7.2-curl 2>> "${logfile}" >/dev/null &

          # Spinning, While the program installs
          spinner  

            pause_webserver PHP, Mariadb 
            echo
            echo -e "\t\t\t\tLet's Secure Your Mariadb Server"
            echo
            mysql_secure_installation
            install_cp_wp

          sudo systemctl start  mariadb 2>> "${logfile}" >/dev/null &

          sudo systemctl enable  mariadb 2>> "${logfile}" >/dev/null &
        #
        #   They didn't want to Install PHP 
        #

      else
            echo
            echo -e "\t\t\t\tCouldn't Secure Mariadb nor install ClassicPress or WordPress"
            return 1

            fi

        return 0
fi
}

install_cp_wp()
{
            
            #
            # Storing ClassicPress Mariabdb Variables To Proceed
            #

            echo "
                 Note: If You are Planning To Migrate or Move an Existing Website To This Server, 
                 Please Make Sure The Database Name, User and Password You are Inputing 
                 Corresponds To What is in your wp-config,
                 Check The wp-config of The Old Website File To Cross Check The Details. 
                 If This is a New Website, Then Create a New Details, Good luck!
                 " | boxes -d columns

            CpDBName=
            while [[ $CpDBName = "" ]]; do
              echo -e "\t\t\t\tEnter ClassicPress or WordPress Database name: \c"
              read CpDBName
            done

            CpDBUser=
            while [[ $CpDBUser = "" ]]; do
              echo -e "\t\t\t\tEnter ClassicPress or WordPress Mysql user: \c"
              read CpDBUser
            done

            CpDBPass=
            while [[ $CpDBPass = "" ]]; do

                while : # Unless Password Matches, Keep Looping
                do

                    echo -e "\t\t\t\tEnter ClassicPress or Wordpress Password For $CpDBUser: \c"
                    read -s CpDBPass # Adding the -s option to read hides the input from being displayed on the screen.
                    echo -e "\t\tRepeat Password: \c"
                    read -s CpDBPass2 # Adding the -s option to read hides the input from being displayed on the screen.
                                # 
                      #   Checking if both passwords match
                      #

                      if [ "$CpDBPass" != "$CpDBPass2" ]; then
                          echo
                          echo -e "\t\t\t\tPasswords do not match, Please Try again"
                      else
                          echo
                          echo -e "\t\t\t\tPasswords Matches, Moving On..." 
                          break
                      fi
                  done # Endwhile loop

            done

            #
            #   Creating ClassicPress DB User and passwords with privileges.
            #

		        	echo -e "\t\t\t\tCreating ClassicPress or WordPress DB Users and granting privileges with already collected information...\n"
			
            #
            #   The s silences errors and the f forces the commands to continue even if one chokes.
            #   The u relates to the username that immediately follows it.
            #
            #

mysql -sfu root <<MYSQL_SCRIPT
CREATE DATABASE $CpDBName DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;
CREATE USER '$CpDBUser'@'localhost' IDENTIFIED BY '$CpDBPass';
GRANT ALL ON $CpDBName.* TO '$CpDBUser'@'localhost' IDENTIFIED BY '$CpDBPass';
FLUSH PRIVILEGES;
MYSQL_SCRIPT
			      #

            #
            # Preparing Temp Directing for Downloading latest Wordpress/ClassicPress tarball and extraction
            #

            TMPDIR=$(mktemp -d /tmp/cp_wp.XXXXXXXXXX) || exit 1
            echo
            echo -e "\t\t\t\tWhat Do You Wish To Do?\n"
            while :
            do
                #
                #   Display the ClassicPress or WordPress Decision Menu
                #
              echo "
                              1.) Install ClassicPress
                              2.) Install Wordpress
                              3.) Restore Website
                              4.) Exit

              " | boxes -d columns

          #  
          #   (Not Relevant Anymore) echo -e "\tType cp For ClassicPress or wp for WordPress: \c"
          #
          echo -e "\t\t\t\tChoose 1 For ClassicPress, 2 for WordPress or 3 to Restore an existing website: \c"
          read cp_wp_decision

          #
          #   Check if User Selected ClassicPress or Wordpress
          #

            case $cp_wp_decision in
              1)
                echo
                echo -e "\t\t\t\tGreat, You Selected ClassicPress\n"
                echo
                if yes_no "Do You Want to Proceed With CP"
                then
                echo
                echo -e "\t\t\t\tWe Proceed with ClassicPress\n"
                echo -e "\t\t\t\tDownloading Latest Classicpress To a Temp Directory"
        
                wget https://www.classicpress.net/latest.tar.gz -O "$TMPDIR"/cplatest.tar.gz 2>> "${logfile}" &

                spinner
                #
                #   Extract the file, and extract it into a folder
                #

                mkdir -p "$TMPDIR"/classicpress && tar -zxf  "$TMPDIR"/cplatest.tar.gz -C "$TMPDIR"/classicpress --strip-components 1 2>> "${logfile}" &

                errorchecker $?

                cp -f "$TMPDIR"/classicpress/wp-config-sample.php "$TMPDIR"/classicpress/wp-config.php &>/dev/null

                errorchecker $?

                cp -a "$TMPDIR"/classicpress/. /var/www/"$websitename" 2>> "${logfile}" &
                
                wait $!

                errorchecker $?

                sudo rm -R "$TMPDIR"

                sudo rm -f /var/www/html/index.nginx-debian.html &
                
                wait $!

                errorchecker $?
                echo
                echo -e "\t\t\t\tAdjusting file and directory permissions..\n"

                #
                #   check if the websitename still has a variable, if no, ask
                #

                while [[ $websitename = "" ]]; do
                echo
                  read -p "Seems, we lost the websitename, re-enter it: " websitename
                done

                errorchecker $?

                #
                #   Change directory and file user and group to www-data
                #

                chown -R www-data:www-data /var/www/"$websitename"

                #
                #   Chnage permission of all directroy and file under websitename
                #

                find /var/www/"$websitename" -type d -exec chmod 755 {} \;

                find /var/www/"$websitename" -type f -exec chmod 644 {} \;

                #
                #   Change permission of wp-config
                #

                chmod 660  /var/www/"$websitename"/wp-config.php

                #
                #   Allow ClassicPress To Manage Wp-content
                #

                find /var/www/"$websitename"/wp-content -type d -exec chmod 775 {} \;
                find /var/www/"$websitename"/wp-content -type f -exec chmod 664 {} \;

                #
                #   Writing ClassicPress config file with collected config data
                #

                echo -e "\t\t\t\tWriting ClassicPress config file with collected config data...\n"

                sed -i "s/database_name_here/$CpDBName/" /var/www/"$websitename"/wp-config.php

                sed -i "s/username_here/$CpDBUser/" /var/www/"$websitename"/wp-config.php

                sed -i "s/password_here/$CpDBPass/" /var/www/"$websitename"/wp-config.php

                progress_bar
                # reload nginx
                sudo systemctl start nginx 2>> "${logfile}" >/dev/null &
                sudo systemctl enable nginx 2>> "${logfile}" >/dev/null &
                sudo systemctl reload nginx 2>> "${logfile}" >/dev/null &
                echo  "
                ClassicPress Installation Has Been Completed Successfully
                Your Error Log file is at  $logfile
                Please browse to http://$websitename/wp-admin/install.php to complete the installation through the web interface
                The information you'll need are as follows:
                1) ClassicPress Database Name: $CpDBName
                2) ClassicPress Database User: $CpDBUser
                3) ClassicPress Database User Password: $CpDBPass
                Save this in a secret place.
                !!
                You can reach me at https://devsrealm.com/
                !!
                Welcome to the ClassicPress communtity, if you need support, please head over to forum.classicpress.net
                " | boxes -d ian_jones

                exit 0
                else 
                return 1
                fi
                ;;
              2)
                echo
                echo -e "\t\t\t\tGood, You Selected Wordpress\n"
                if yes_no "Do You Want to Proceed With WP"
                then
                echo
                echo -e "\t\t\t\tWe Proceed with WordPress"
                echo -e "\t\t\t\tDownloading Latest WordPress To a Temp Directory"
                spinner
                wget https://wordpress.org/latest.tar.gz -O "$TMPDIR"/wplatest.tar.gz 2>> "${logfile}" &

                spinner
                #

                #
                #   Extract the file, and extract it into a folder
                #
                mkdir -p "$TMPDIR"/wordpress && tar -zxf  "$TMPDIR"/latest.tar.gz -C "$TMPDIR"/wordpress --strip-components 1 2>> "${logfile}" &
                
                wait $!
                errorchecker $?

                cp -f "$TMPDIR"/wordpress/wp-config-sample.php "$TMPDIR"/wordpress/wp-config.php 2>> "${logfile}" &

                wait $!
                errorchecker $?

                cp -a "$TMPDIR"/wordpress/. /var/www/"$websitename" 2>> "${logfile}" &

                wait $!
                errorchecker $?

                sudo rm -R "$TMPDIR"

                sudo rm -f /var/www/html/index.nginx-debian.html 2>> "${logfile}" &

                wait $!
                errorchecker $?
                echo
                echo -e "\t\t\t\tAdjusting file and directory permissions..\n"

                #
                #   check if the websitename still has a variable, if no, ask
                #

                while [[ $websitename = "" ]]; do
                echo
                  read -p "Seems, we lost the websitename, re-enter it: " websitename
                done

                errorchecker $?

                #
                #   Chnage directory and file user and group to www-data
                #

                chown -R www-data:www-data /var/www/"$websitename"

                #
                #   Chnage permission of all directroy and file under websitename
                #

                find /var/www/"$websitename" -type d -exec chmod 755 {} \;

                find /var/www/"$websitename" -type f -exec chmod 644 {} \;

                #
                #   Chnage permission of wp-config
                #

               chmod 660  /var/www/"$websitename"/wp-config.php

                #
                #   Allow WordPress To Manage Wp-content
                #

                find /var/www/"$websitename"/wp-content -type d -exec chmod 775 {} \;
                find /var/www/"$websitename"/wp-content -type f -exec chmod 664 {} \;

                #
                #   Writing Wordpress config file with collected config data
                #
                echo
                echo -e "\t\t\t\tWriting Wordpress config file with collected config data...\n"

 
                sed -i "s/database_name_here/$CpDBName/" /var/www/"$websitename"/wp-config.php

                sed -i "s/username_here/$CpDBUser/" /var/www/"$websitename"/wp-config.php

                sed -i "s/password_here/$CpDBPass/" /var/www/"$websitename"/wp-config.php

                sudo systemctl start nginx 2>> "${logfile}" >/dev/null &
                sudo systemctl enable nginx 2>> "${logfile}" >/dev/null &
                sudo systemctl reload nginx 2>> "${logfile}" >/dev/null &
                
                progress_bar
                echo  "
                Wordpress Installation has been completed successfully
                Your Error Log file is at  $logfile
                Please browse to http://$websitename/wp-admin/install.php to complete the installation through web interface
                The information you'll need are as follows:
                1) Wordpress Database Name: $CpDBName
                2) Wordpress Database User: $CpDBUser
                3) Wordpress Database User Password: $CpDBPass
                Save this in a secret place.
                !!
                You can reach me at https://devsrealm.com/
                !!
                " | boxes -d ian_jones

                exit 0
                else 
                return 1
                fi
               ;;
               3) 
                echo
                echo -e "\t\t\t\tGood, You Selected Restore\n"
                echo
                if yes_no "Do You Want to Proceed With Restoring an Existing Website "
                then
                #
                #   We would be using atool for the extraction, I choosed this external program
                #   because sometimes user might want to extract format other than tar.gz. So, 
                #   even if you want to extract a rar file, it still works, even .7z, cool right :)
                #

                    if command -v atool 2>> "${logfile}" >/dev/null # Checking if the atool package is installed
                    then
                      echo

                      echo -e "\t\t\t\tatool dependency Okay...."

                    else

                      echo -e "\t\t\t\tInstalling Dependencies...."
                      sudo apt-get -y install atool 2>> "${logfile}" >/dev/null &
                      spinner
                      echo -e "\t\t\t\tatool dependency Installed, Moving On...." 

                    fi # End Checking if the atool package is installed

                 TMPDIR=$(mktemp -d /tmp/website_restore_XXXXX) || exit 1

                 while :
                 do

                 echo "
                 Note: You Can Either Pass a Filename Located in The Current Directory, 
                 e.g file.zip or Specify The Directory In Which The File is Located, 
                 and Point To it, e.g /path/to/directory/file.zip
                      " | boxes -d columns

                  read -p  $'\t\t\t\t'"The Name of Your Compressed File: " website_restore

                  echo "
                  Note: You Can Either Pass a Database Located in The Current Directory, 
                  e.g database.sql or Specify The Directory In Which The Database is Located, 
                  and Point To it, e.g /path/to/directory/database.sql
                      " | boxes -d columns

                  read -p  $'\t\t\t\t'"The Name of Your Database File (This should be in .sql): " db_file

                  #
                  #   Getting The Extension of the db_file, in order to run a test if it is in .sql
                  #

                  stripfilename=$(basename -- "$db_file") # This gets the filename without the path

                  extension="${stripfilename##*.}" # This Extracts the extension, in which case, we are looking for .sql

                  #
                  #   Checking if the both file exist and the db extension ends in sql
                  #

                  if [[ ! -f $website_restore && ! -f $website_restore && $extension != "sql" ]]
                  then

                      echo -e "\t\t\tYou are either not referencing a correct archive or not referencing an actual sql file, Please Point To an Actual File"

                  else

                      echo -e "\t\t\t\tGreat, Directory and Database file Exist... Moving On" 
                      echo
                      break    

                  fi # END [[ ! -f $website_restore && ! -f $website_restore && $extension != "sql" ]]
               done # Endwhile loop

                  echo -e "\t\t\t\tExtracting Into a Temp Directory"

                  aunpack "$website_restore" -X "$TMPDIR" 2>> "${logfile}" >/dev/null

                  echo -e "\t\t\t\tChecking if You Have Unneccesary folder"


                  #
                  #   Some Users Would Have Their Archive File Contain Another Folder, e.g If The Folder is archive.zip
                  #   when extracted, it might turn out that their is another folder in the folder, e.g /archive/public_html
                  #   In this case, the below code would check if there is any folder at all, if there is none, we break out of the loop
                  #
                  #   If there is one, we copy everything recursively into the main folder, and we delete the empty folder
                  #

                  cd "$TMPDIR" || { echo -e "\t\t\t\tCouldn't Change into Temp Directory...Exiting"; return 1; }

                  #
                  #   This code was originally for i in $(ls), which should be avoidable since 
                  #   Using command expansion causes word splitting and glob expansion e.g a file with spaces, and other weird file naming
                  #
                  #   I changed this to for i in $TMPDIR, but really it really doesn't matter in this case as I have already handle the cases of no
                  #   matches in the code below
                  #
                  
                  for i in $TMPDIR

                  do

                    if [ ! -d "$i" ]  # If there is no folder, break out
                    then
                    echo -e "\t\t\t\tYou Don't Have an Unnecessary Folder, Moving On."
                    break

                    else # If there is a folder, copy all of its content into the main root folder

                    echo -e "\t\t\t\tYou have an unnecessary folder, next time, make sure you are not archiving a folder along your files, \n\t\t\t\tLet me take care of that for you..."

                    \cp -a "$i"/. "$TMPDIR"/

                    fi

                  done

                  #
                  # Go back into the script directory
                  #
                  SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")
                  cd "$SCRIPT_DIR"

                  spinner

                  if yes_no "Before We Move On, Would You Like To Check If The wp-config data Contains The Actual DB Details "
                  then

                      if [ ! -f "$TMPDIR"/wp-config.php ]
                          then
                              echo -e "\t\t\twp-config.php is missing, Looks like you extracted the wrong file!"
                              return 1
                          else

                            nano "$TMPDIR"/wp-config.php

                            echo -e "\t\t\t\tMoving Into The Real Directory"

                            cp -a "$TMPDIR"/. /var/www/"$websitename" 2>> "${logfile}" >/dev/null

                            errorchecker_restore $?

                            sudo rm -R "$TMPDIR"

                            echo

                            echo -e "\t\t\t\tAdjusting file and directory permissions..\n"

                          #
                          #   Change directory and file user and group to www-data
                          #

                          chown -R www-data:www-data /var/www/"$websitename"

                          #
                          #   Chnage permission of all directroy and file under websitename
                          #

                          find /var/www/"$websitename" -type d -exec chmod 755 {} \;

                          find /var/www/"$websitename" -type f -exec chmod 644 {} \;

                          #
                          #   Chnage permission of wp-config
                          #

                          chmod 660  /var/www/"$websitename"/wp-config.php

                          #
                          #   Allow ClassicPress To Manage Wp-content
                          #

                          find /var/www/"$websitename"/wp-content -type d -exec chmod 775 {} \;
                          find /var/www/"$websitename"/wp-content -type f -exec chmod 664 {} \;



                           echo -e "\t\t\t\tFinalizing Restoration...\n"

                           mysql --user="$CpDBUser" --password="$CpDBPass" "$CpDBName" < "$db_file"

                          progress_bar
                          # reload nginx
                          sudo systemctl reload nginx &>/dev/null

                          echo "
                          $websitename restored, Check if you can access the website, 
                          and you might also want to secure it using the Free Let's Encrypt SSL
                              " | boxes -d columns

                          return 0


                      fi # END [ ! -f "$TMPDIR"/wp-config.php ]

                  #
                  #   They didn't want to open wp-config, so, we move on...
                  #

                  else

                  echo -e "\t\t\t\tMoving Into The Real Directory"

                  cp -a "$TMPDIR"/. /var/www/"$websitename" 2>> "${logfile}" >/dev/null

                  errorchecker_restore $?

                  sudo rm -R "$TMPDIR"

                  echo

                  echo -e "\t\t\t\tAdjusting file and directory permissions..\n"

                #
                #   Change directory and file user and group to www-data
                #

                chown -R www-data:www-data /var/www/"$websitename"

                #
                #   Chnage permission of all directroy and file under websitename
                #

                find /var/www/"$websitename" -type d -exec chmod 755 {} \;

                find /var/www/"$websitename" -type f -exec chmod 644 {} \;

                #
                #   Chnage permission of wp-config
                #

               chmod 660  /var/www/"$websitename"/wp-config.php

                #
                #   Allow ClassicPress To Manage Wp-content
                #

                find /var/www/"$websitename"/wp-content -type d -exec chmod 775 {} \;
                find /var/www/"$websitename"/wp-content -type f -exec chmod 664 {} \;

                echo -e "\t\t\t\tFinalizing Restoration...\n"

                mysql --user="$CpDBUser" --password="$CpDBPass" "$CpDBName" < "$db_file"

                progress_bar
                # reload nginx
                sudo systemctl reload nginx 2>> "${logfile}" >/dev/null &

                echo "
                     $websitename restored, Check if you can access the website, 
                     and you might also want to secure it using the Free Let's Encrypt SSL
                     " | boxes -d columns

                fi # END "Before We Move On, Would You Like To Check If The wp-config data Contains The Actual DB Details "

              return 0

           fi # END Do You Want to Proceed With Restoring an Existing Website
                ;;
               4) 
                return 0
               ;;
            *)
                echo
                echo -e "\tplease enter a number between 1 and 4"
                pause
                echo
                ;;
               
                #
                # and continue around the loop ....
                #
        esac
done
            # remove the tempdirectory
            sudo rm -rf "$TMPDIR"

}

#
#   sFTP is a “secure version of file transfer protocol which helps
#   in transmitting data over a secure shell data stream, it is simply a secure way of transferring
#   files between local and remote servers.
#
#   This function helps to automate the creation of sftp user group, this way, we can restrict
#   this group to the each user document root e.g /var/www/website1
#

sftp(){

  # 
  #   Read more about managing users in Linux
  #
  #   https://devsrealm.com/cloud-computing/ubuntu/an-easy-guide-to-managing-users-in-ubuntu-18-04/
  #
  #   The goal of this function is not to create an sftp access arbitrarily, but to
  #   to jail a user, this way they have no way to break out of their home directory.
  #
  #   We then mount a location to their home directory, so they can upload files to their website root folder.
  #   For example, If user "paul" needs to upload files to /var/www/websitename.com, we chroot to /home/jail/paul
  #   and then mount /var/www/websitename.com to their home directory, this way, they can upload to the correct website directory without 
  #   needing access to it, and thus, things would be breeze and secure.
  #
  SFTP_ONLY_GROUP="sftp_only"

  # Make sure the group exists
  grep  -i "^${SFTP_ONLY_GROUP}" /etc/group  >/dev/null 2>&1

  if [ $? -eq 0 ]; then # If the above command return true, that is, if group exist
          echo -e "\t\t\tInitial Setup Okay... Moving On."
  else
          echo -e "\t\t\t\tAdding Initials"
          sudo groupadd $SFTP_ONLY_GROUP # Create sftp group

          #
          #   Disabling normal sftp and enabling jailed sftp
          #
          config='/etc/ssh/sshd_config'
          sed -i "s/Subsystem.*sftp/#Subsystem sftp/" /etc/ssh/sshd_config # Disabling normal sftp

          #   Enabling Jailed SFTP
          echo " " >> $config
          echo "Subsystem sftp internal-sftp" >> $config
          echo "Match Group $SFTP_ONLY_GROUP" >> $config
          echo "ChrootDirectory %h" >> $config
          echo "    AllowTCPForwarding no" >> $config
          echo "    X11Forwarding no" >> $config
          echo "    ForceCommand internal-sftp" >> $config

  fi

  #
  #   Creating a chroot Directory, this is the home directory we will be chrooting our sFTP user to
  #   This would enable us have more directories with each one relating to a different website.
  #

  chroot_dir=/sftpusers/jailed


  echo -e "\t\t\t\tEnter SFTP New User: \c"
  read sftp_user

# Does User exist?
id "$sftp_user" &> /dev/null
if [ $? -eq 0 ]; then
    echo -e "\t\t\t$sftp_user exists"
        if yes_no "Do you want to change password instead?"
          then

          while : # Unless Password Matches, Keep Looping
          do
              echo -e "\t\t\tEnter $sftp_user new password: \c"
              read -s pass1 # Adding the -s option to read hides the input from being displayed on the screen.
              echo -e "\t\t\tRepeat $sftp_user new password: \c"
              read -s pass2

              # 
              #   Checking if both passwords match
              #

              if [ "$pass1" != "$pass2" ]; then
                  echo
                  echo -e "\t\t\t\tPasswords do not match, Please Try again"
              else
                  echo
                  echo -e "\t\t\t\tPasswords Matches, Moving On..." 
                  break
              fi
          done # Endwhile loop

          echo "$sftp_user:$pass2" | chpasswd -c SHA512 #Encrypt Password using SHA512  
          echo -e "\t\t\tPassword Changed"
          return 0
      fi

else

  if [ ! -d $chroot_dir/"$sftp_user" ];then
    echo -e "\t\t\t\t$sftp_user directory and user does not exist yet...creating"

    mkdir -p $chroot_dir/"$sftp_user"/
    mkdir -p $chroot_dir/"$sftp_user"/"$websitename"
    echo -e "\t\t\t\t$sftp_user created"
  fi  

    #
    #   Adding the new user with the home directory with no ability to shell login  
    #   The user won't be able to SSH into the server, but can only access through SFTP for transferring files
    #

    useradd -d $chroot_dir/"$sftp_user"/ -s /usr/sbin/nologin -G $SFTP_ONLY_GROUP "$sftp_user"

    while : # Unless Password Matches, Keep Looping
    do

    echo -e "\t\t\t\tEnter $sftp_user new password: \c"
    read -s pass1 # Adding the -s option to read hides the input from being displayed on the screen.
    echo
    echo -e "\t\t\t\tRepeat $sftp_user new password: \c"
    read -s pass2

        # 
        #   Checking if both passwords match
        #

        if [ "$pass1" != "$pass2" ]; then
            echo
            echo -e "\t\t\t\tPasswords do not match, Please Try again"
        else
            echo
            echo -e "\t\t\t\tPasswords Matches, Moving On..." 
            break
        fi
    done # Endwhile loop

    echo
    echo -e "\t\t\t\tChanging password\n"
    echo "$sftp_user:$pass2" | chpasswd -c SHA512 #Encrypt Password using SHA512  

    #
    #   Restart SSH
    #

    service ssh restart >/dev/null 2>&1
    service sshd restart >/dev/null 2>&1

    echo -e "\t\t\t\tSetting Proper User Permissions..."

    chmod 711 $chroot_dir
    chmod 755 $chroot_dir/"$sftp_user"/
    chown root:root $chroot_dir # chowning the chroot_directory

    #   Setting the permissions of the mount directory

    chown "$sftp_user":$SFTP_ONLY_GROUP $chroot_dir/"$sftp_user"/"$websitename"/
    chmod 700 $chroot_dir/"$sftp_user"/"$websitename"/

    #
    #   The below forces any new files or directories created by this user to have a group matching the parent directory
    #

    find /var/www/"$websitename"/ -type d -exec chmod g+s {} \;

    #
    #   Now we mount a specific directory to the users chrooted home directory.
    #   The below command mount the directory stated in /etc/fstab to the directory we have just specified 
    #   ($chroot_dir/$sftp_user/$websitename/)
    #

      mount -o bind /var/www/"$websitename" $chroot_dir/"$sftp_user"/"$websitename"

    #
    #   The /etc/fstab file is a system configuration file that contains all 
    #   available disks, disk partitions and their options. 
    #   Each file system is described on a separate line. This file would help to mount additional volumes you would like to 
    #   automatically mount at boot time
    #

    echo -e "\t\t\t\tMounting Directory..."
    echo "/var/www/$websitename/ $chroot_dir/$sftp_user/$websitename/ none bind 0 0" >> /etc/fstab

     echo -e "\t\t\t\tSFTP User Created"

    #
    #   Restart SSH
    #

    service ssh restart >/dev/null 2>&1
    service sshd restart >/dev/null 2>&1

    echo  "
    SFTP User Created
    You can login with $sftp_user@$ip and your choosen password
    " | boxes -d ian_jones

     return 0

  fi
}

#
#   DNS or Domain Name System is a service that is responsible for IP address translation
#   to a domain or hostname. It is much easier to connect and remember domain.com, than it
#   is to remember its IP address. When you connect to the internet, your server will connect
#   to an external DNS server (which we would be configuring below) in order to figure out the
#   IP addresses for the website you want to visit.
#
#   If your domain registrar doesn't provide you a free DNS server, then use this function, this way,
#   you would be able to create a custom DNS record, in other words, you are basically hosting your own DNS server
#

dns()
{
    #
    #   Berkeley Internet Name Domain or Bind is a service that allows the publication
    #   of DNS information on the internet, it also faciliate the resolving of DNS queries
    #   Since Bind is the most popular DNS program, this is what we would be using.
    #
    if command -v named 2>> "${logfile}" >/dev/null
    then
      echo

      echo -e "\t\t\t\tBind9 Okay...."

    else

      echo -e "\t\t\t\tInstalling Bind9...."
      sudo apt-get -y install bind9 dnsutils 2>> "${logfile}" >/dev/null &
      spinner

      echo -e "\t\t\t\tBind9 Installed, Moving On...." 
    fi   

    named_local=/etc/bind/named.conf.local
    db_local=/etc/bind/db.local

    #
    #   This would store our custom_nameserver file
    #
    customnameserver=custom_nameserver

    #
    #   Check if the filename represents a valid file.
    #
    if [ ! -f $customnameserver ]
      then
          echo -e "\t\t\t\tYou Haven't Choosen Your Custom NameServer"

          #
          #   Ask if it should be created
          #
        if yes_no "Do You Want To Set a Custom NameServers"
          then

          #
          #   Attempt to create it
          #
          while : # Unless NameServer Matches, Keep Looping
          do
          read -p  $'\t\t\t\t'"What Domain Name Do You Want To Use as Your Custom NameServers, e.g example.com: " nameserver
          read -p  $'\t\t\t\t'"Enter Domain Name Again: " nameserver2

          # 
          #   Checking if the domain name matches
          #

          if [ "$nameserver" != "$nameserver2" ]; then
              echo -e "\t\t\t\tDomain name do not match, Please Try again"
          else
              echo -e "\t\t\t\tDomain Matches, Moving On..." 
              echo
echo "ns1.$nameserver
ns2.$nameserver" >  $customnameserver

echo -e "\t\t\t\tSetting Custom NameServer"

          #
          #   Store The Both Custom NameServers
          #

          ns1=$(sed -n '1p' < "$customnameserver")
          ns2=$(sed -n '2p' < "$customnameserver")

          TMPFILE=$(mktemp /tmp/named.conf.XXXXXXX) || exit 1
          cat $named_local > "$TMPFILE"
          #
          #   A zone is a domain name that is referenced in the DNS server.
          #
      echo -e "\t\t\t\tSetting Zone File"
echo "// Forward Zone File of $websitename
zone \"$nameserver\" {
      type master;
      file \"/etc/bind/db.$nameserver\";
};" >> "$TMPFILE"
          
          sudo cp -f "$TMPFILE" $named_local # move the temp to nginx.conf
          # remove the tempfile
          rm "$TMPFILE"

          zone_file=/etc/bind/db.$nameserver

          echo -e "\t\t\t\tPreparing Zone File\n"
          TMPFILE=$(mktemp /tmp/zone.conf.XXXXXXX) || exit 1
          sudo cp $db_local "$TMPFILE"


          date=$(date +"%Y%m%d")

          rootemail=$(cat "$customnameserver" | sed -n '1s/ns1./root./p')
          #
          #   Get Server IP
          #
          ip="$(ip route get 8.8.8.8 | sed -n '/src/{s/.*src *\([^ ]*\).*/\1/p;q}')"
echo ";
; BIND data file for $nameserver
;
\$TTL    604800
@       IN      SOA      $ns1.          $rootemail. (
                         $date"00"      ; Serial
                         604800         ; Refresh
                          86400         ; Retry
                        2419200         ; Expire
                         604800 )       ; Negative Cache TTL
;
@       IN      NS      $ns1.
@       IN      NS      $ns2.
@       IN      A       $ip
ns1     IN      A       $ip
ns2     IN      A       $ip
www     IN      A       $ip
ftp     IN      A       $ip
mail    IN      A       $ip
smtp    IN      A       $ip
pop     IN      A       $ip
imap    IN      A       $ip
@       IN      TXT    \"v=spf1 a mx ip4:$ip ~all\"
_dmarc  IN      TXT     \"v=DMARC1; p=quarantine; pct=100\"" > "$TMPFILE"

          sudo cp -f "$TMPFILE" /etc/bind/db."$nameserver"
          sudo chmod 640 /etc/bind/db."$nameserver"

          # remove the tempfile
          rm "$TMPFILE"

          echo -e "\t\t\t\tRestarting Services\n"
          service bind9 restart

          progress_bar

          echo "
                Your Custom NameServer is $ns1 and $ns2
                The DNS Server Won't Work Until You Change 
                Your Domain NameServer Via Your Domain's Registrar Website

                Also, You Don't Need To Create a Zone For The Custom Nameserver Domain Name 
                (i.e $nameserver)
                Although, You Can Edit The Zone or Add A New Zone Domain Name.

                " | boxes -d columns


          break
          fi  # End Checking if the domain name matches

          done # Endwhile loop

        fi # End Set a Custom NameServers

        return 0

      fi #    End Check if the filename represents a valid file.
      
    echo -e "\t\t\t\tInitials Okay, Moving On...\n"
    while :
          do
          #
          #   Display DNS Menu
          #
                  echo "
                        1.) Add New Zone to DNS server 
                        2.) Edit DNS Zone
                        3.) Delete DNS Domain
                        4.) Exit

                        " | boxes -d columns

                #
                #   Prompt for an answer
                #
                echo -e "\t\t\t\tAnswer (or 'q' to quit): \c?"
                read ans junk

                #
                #   Empty answers (pressing ENTER) cause the menu to redisplay,
                #   so, this goes back around the loop
                #   We only make it to the "continue" bit if the "test"
                #   program ("[") returned 0 (True)
                #
                [ "$ans" = "" ] && continue

                #
                #   Decide what to do base on user selection
                #

                    case $ans in
                    1)     zone_add
                    ;;
                    2)     zone_edit
                    ;;
                    3)     zone_delete
                    ;;
                    4)     quit 0
                    ;;
                    q*|Q*) quit 0
                    ;;
                    *)     echo -e "\t\t\t\tPlease Enter a Number Between 1 and 4";;
                esac
                #
                #   Pause to give the user a chance to see what's on the screen, this way, we won't lose some infos
                #
                pause

    done              
}

zone_add ()

{
  echo
  #
  #   $'\t' is an ANSI-C quoting, this would make us tab the read prompt, instead of relying on echo
  #   I should probaly change the rest of the code to follow this syntax
  #
  echo -e "\t\t\tYour Domain Name is $websitename\n"
  if yes_no "Is That Correct "
      then
      echo -e "\t\t\t$websitename Has Been Choosen as Your Domain Zone Name"
      read -p  $'\t\t\t\t'"Enter IP address Of Domain (The IP of the Server Hosting The Domain): " DomainIP

  else
      read -p  $'\t\t\t\t'"Enter Domain Name (FQDN), e.g, example.com: " websitename
      read -p  $'\t\t\t\t'"Enter IP address Of Domain (The IP of the Server Hosting The Domain): " DomainIP

    named_local=/etc/bind/named.conf.local
    db_local=/etc/bind/db.local

    TMPFILE=$(mktemp /tmp/named.conf.XXXXXXX) || exit 1
    cat "$named_local" > "$TMPFILE"
    #
    #   A zone is a domain name that is referenced in the DNS server.
    #
echo "
// Forward Zone File of $websitename
zone \"$websitename\" {
    type master;
    file \"/etc/bind/db.$websitename\";
};" >> "$TMPFILE"
    
    sudo cp -f "$TMPFILE" $named_local # move the temp to nginx.conf
    # remove the tempfile
    rm "$TMPFILE"

    zone_file=/etc/bind/db.$websitename

    echo -e "\t\t\t\tPreparing Zone File\n"
    TMPFILE=$(mktemp /tmp/zone.conf.XXXXXXX) || exit 1
    sudo cp $db_local "$TMPFILE"


    date=$(date +"%Y%m%d")

    rootemail=$(cat custom_nameserver | sed -n '1s/ns1./root./p')
    #
    #   Store The Both Custom NameServers
    #

    ns1=$(sed -n '1p' < "$customnameserver")
    ns2=$(sed -n '2p' < "$customnameserver")

    #
    #   Get Server IP
    #
    ip="$(ip route get 8.8.8.8 | sed -n '/src/{s/.*src *\([^ ]*\).*/\1/p;q}')"

echo ";
; BIND data file for $websitename
;
\$TTL    604800
@       IN      SOA      $ns1.          $rootemail. (
                         $date"00"      ; Serial
                         604800         ; Refresh
                          86400         ; Retry
                        2419200         ; Expire
                         604800 )       ; Negative Cache TTL
;
@       IN      NS      $ns1.
@       IN      NS      $ns2.
@       IN      A       $DomainIP
www     IN      A       $DomainIP
ftp     IN      A       $DomainIP
mail    IN      A       $DomainIP
smtp    IN      A       $DomainIP
pop     IN      A       $DomainIP
imap    IN      A       $DomainIP
@       IN      TXT    \"v=spf1 a mx ip4:$ip ~all\"
_dmarc  IN      TXT     \"v=DMARC1; p=quarantine; pct=100\"" > "$TMPFILE"

    sudo cp -f "$TMPFILE" /etc/bind/db."$websitename"
    sudo chmod 640 /etc/bind/db."$websitename"

    # remove the tempfile
    rm "$TMPFILE"

    echo -e "\t\t\t\tRestarting Services\n"
    service bind9 restart



  fi    
}

zone_edit () {
  if yes_no "You Are About To Edit $websitename Zone File, is That Correct "
  then
    nano /etc/bind/db."$websitename"
    #
    #   The Serial in the zone file is one of the record that would frustrate you like hell
    #   If you are doing things manually, the reason is because it's not enough to just update the  
    #   zone file any time we make a change to it, you also need to remember to increase the serial number by at least one.
    #
    #   Without Doing That, There is no way bind would know you updated anything, well, that is how it works
    #   The below code would extract the seril number, delete the word seria (this is actually a comment, not useful)
    #   and then increment it by 1 anytime we make changes, cool right
    #
    #
    oldserial=$(cat /etc/bind/db."$websitename" | sed -n '6s/; Serial//p')
    newserial=$(expr "$oldserial" + 1)

    sed -i "s/$oldserial/\\t\t\\t$newserial\\t/" /etc/bind/db."$websitename"
    echo -e "\t\t\t\tRestarting Services\n"
    service bind9 restart

    return 0

   else

    read -p  $'\t\t\t\t'"Enter Website You Would Like To Edit Its Zone: " zonewebsite

    if [ ! -f /etc/bind/db."$zonewebsite" ]
      then

          echo -e "\t\t\tThere is no such zone file"
          return 1
    else

        nano /etc/bind/db."$zonewebsite"

        oldserial=$(cat /etc/bind/db."$websitename" | sed -n '6s/; Serial//p')
        newserial=$(expr "$oldserial" + 1)

        sed -i "s/$oldserial/\\t\t\\t\t\\t\t\\t\t\\t\t\\t\t\\t$newserial\\t\t/" /etc/bind/db."$websitename"
        echo -e "\t\t\t\tRestarting Services\n"
        service bind9 restart

        return 0

    fi

  fi
}

zone_delete ()
{
  read -p  $'\t\t\t\t'"Enter Website of The Zone You Would Want Deleted: " zonewebsite

  if [ ! -f /etc/bind/db."$zonewebsite" ]
      then

          echo -e "\t\t\tThere is no such zone file"
          return 1
  else  

    if yes_no "Are You Sure About The Zone Deletion of $zonewebsite "
    then
    TMPFILE=$(mktemp /tmp/delete."$zonewebsite".XXXXX) || exit 1
    sudo cp $named_local "$TMPFILE"
    sed -nie "/\"$zonewebsite\"/,/^\};"'$/d;p;' TMPFILE

    sudo cp -f "$TMPFILE" $named_local

    rm /etc/bind/db."$zonewebsite"

    # remove the tempfile
    rm "$TMPFILE"

    echo -e "\t\t\t\tRestarting Services\n"
    service bind9 restart

    pause

    return 
    fi
fi
}

#
#   phpMyAdmin is a free software tool written in PHP, intended to handle the administration of MySQL over the Web. 
#   phpMyAdmin supports a wide range of operations on MySQL and MariaDB. 
#   Frequently used operations (managing databases, tables, columns, relations, indexes, users, permissions, etc) 
#   can be performed via the user interface, while you still have the ability to 
#   directly execute any SQL statement.
#
#   This function would automate the installation of phpMyAdmin, phpmyadmin would also configure the database for us, so, you don't 
#   need to create any additonal db
#
#

phpmyadmin()
{

dpkg -s phpmyadmin &> /dev/null  # Checking The Status of phpMyAdmin

if [ $? -ne 0 ] # If phpMyAdmin is not installed
then
echo
echo -e "\t\t\t\tphpmyadmin not installed...."
echo -e "\t\t\t\tInstalling...."
DEBIAN_FRONTEND=noninteractive apt-get -yq install phpmyadmin 2>> "${logfile}" >/dev/null & # Install it without prompting interactive
spinner

echo -e "\t\t\t\tUbuntu 18.04 Ships With phpmyadmin 4.6.6, Let's Remove That and Download The Latest Version.."
echo -e "\t\t\t\tas This can fix a couple of Errors When Using phpmyadmin"

wget -O phpmyadmin_5.0.zip https://files.phpmyadmin.net/phpMyAdmin/5.0.0/phpMyAdmin-5.0.0-all-languages.zip 2>> "${logfile}" >/dev/null

echo -e "\t\t\t\tunzip is required for extraction, checking if it is installed...."

if command -v unzip 2>> "${logfile}" >/dev/null # Checking if the unzip package is installed
then
    echo
    echo -e "\t\t\t\tUnzip Okay...."

else

    echo -e "\t\t\t\tUnzip isn't installed, this is required for the extraction. installing...."
    sudo apt-get -y install unzip 2>> "${logfile}" >/dev/null &
    spinner
    echo -e "\t\t\t\tUnzip Installed, Moving On...." 

fi # End Checking if the unzip package is installed

echo -e "\t\t\t\tUnzipping The New phpmyadmin ...."

unzip phpmyadmin_5.0.zip 2>> "${logfile}" >/dev/null &
spinner

echo -e "\t\t\t\tBacking up The Former phpMyAdmin files"

sudo mv /usr/share/phpmyadmin /usr/share/phpmyadmin-backed

echo -e "\t\t\t\tMove The new phpmyadmin to /usr/share/phpmyadmin/ directory"
sudo mv phpMyAdmin-5.0.0-all-languages /usr/share/phpmyadmin

#
#   Editing The Config To Point to The Right File
#

vendor_config=/usr/share/phpmyadmin/libraries/vendor_config.php

sed -i "s/define('CONFIG_DIR', ROOT_PATH);/define('CONFIG_DIR', '\/etc\/phpmyadmin\/');/" $vendor_config

echo -e "\t\t\t\tCreating a Cache Directory"

sudo mkdir /usr/share/phpmyadmin/tmp
sudo chown www-data:www-data /usr/share/phpmyadmin/tmp

echo -e "\t\t\t\tphpmyadmin installed..."

echo -e "\t\t\t\tLet's Create a New Administrative User Account For Your DATABASES"

#
# Storing phpMyAdmin Mariabdb Variables To Proceed
#

pmauser=
while [[ $pmauser = "" ]]; do
  echo -e "\t\t\t\tEnter New phpMyAdmin Username: \c"
  read pmauser
done

pmapass=
  while [[ $pmapass = "" ]]; do

    while : # Unless Password Matches, Keep Looping
    do

        echo -e "\t\t\t\tEnter a Strong Password For $pmauser: \c"
        read -s pmapass # Adding the -s option to read hides the input from being displayed on the screen.
        echo -e "\t\tRepeat Password: \c"
        read -s pmapass2 # Adding the -s option to read hides the input from being displayed on the screen.
                      # 
                      #   Checking if both passwords match
                      #

          if [ "$pmapass" != "$pmapass2" ]; then
            echo
            echo -e "\t\t\t\tPasswords do not match, Please Try again"
          else
            echo
            echo -e "\t\t\t\tPasswords Matches, Moving On..." 
          break
          fi
    done # Endwhile loop

  done

mysql -sfu root <<MYSQL_SCRIPT
GRANT ALL ON *.* TO '$pmauser'@'localhost' IDENTIFIED BY '$pmapass';
FLUSH PRIVILEGES;
MYSQL_SCRIPT

echo "
     Note: Input a single word below, e.g if you would love to login to your phpmyadmin 
     through myphpmyadmin.$websitename then just enter myphpmyadmin and
     I'll take care of the rest :), Again, make sure you enter a single word
     " | boxes -d columns

while :
do

  echo -e "\t\t\t\tYour preffered phpmyadmin url name \c"
  read pmaurl

  #
  #   Checking if user actually passes one argument
  #
  #   We created an array out of the input, and checked the words in there, if it is
  #   more than one, we warn the user, if it is only one, we break
  #

 pmaArray=($pmaurl)

    if [[ ${#pmaArray[@]} -eq 1 ]]
    then
    break
    else
    echo "Please, Enter a Single word"
    fi

done

TMPFILE=$(mktemp /tmp/pma.nginx.XXXXXXXX) || exit 1

    #
    #   This code was originally cat pma_nginx_config | sudo sed -e "s/phpmyadmin.com/$pmaurl.$websitename/g" > "$TMPFILE"
    #   Which is wrong and known as the useless use of cat, It's more efficient and less roundabout to simply use redirection.
    #
    #   So, what I did here was first redirecting the content of < "pma_nginx_config" to sed program, I then redirect the output 
    #   of whatever I get to the > TMPFILE
    #

    sed -e "s/phpmyadmin.com/$pmaurl.$websitename/g" < pma_nginx_config > "$TMPFILE"

    sudo cp -f "$TMPFILE" /etc/nginx/sites-enabled/phpmyadmin

    # remove the tempfile
    rm "$TMPFILE"

    # reload nginx
    sudo systemctl enable nginx  2>> "${logfile}" >/dev/null &
    sudo systemctl reload nginx 2>> "${logfile}" >/dev/null &

    if yes_no "Do you want to secure the phpmyadmin dashboard with Let's Encrypt"
       then

        echo -e "\t\t\t\tYour Email Address: \c"
        read email
        certbot --nginx -d "$pmaurl.$websitename"-d -m "$email" --agree-tos --redirect --hsts --staple-ocsp 2>> "${logfile}" >/dev/null &
        errorchecker_certbot $?
        echo
        echo -e "\t\t\t\tDone\n"

    fi


echo "
     You can login via $pmaurl.$websitename using $pmauser and $pmapass, 
     Save this in a secret place
     The next you would need to do is to point A record to $pmaurl.$websitename, 
     you can do that using the Manage DNS in the main menu easily.
     " | boxes -d columns

else
echo -e "\t\t\t\tphpmyadmin installed...."
fi

}

#####################################
#   WE START THE MAIN CODE HERE ;)  #
#####################################

#
#   This checks if one and only arguments is passed
#   "$#" contains the number of command line arguments.
#   The "$0" is the paramter that comes before other parameters, so, we can say...
#   "$0" is the name of the  script (complete path as
#   typed in on the command line (e.g. "/home/mark/menu")
#
echo
[ $# == 1 ] || usage "$0" websitename.com         # exits the program
echo


logfile=errorlog.txt

sudo apt-get -y install boxes 2>> ${logfile} >/dev/null &
spinner

echo "

                A CommandLine Control Panel That Handles The Automation of 
                ClassicPress, WordPress, and also Offers The Ability To Manage Your 
                Own Custom DNS Directly From The Server.

                That is Not All, You Can Also Install PhpMyAdmin For MariaDB Database, 
                Add Multiple SFTP Users, 
                Automate The Backing Up (deduplicates + You can configure the frequency of the backup)
                and Restoration of The Complete sites 
                (or individual components, e.g bind, SSL cert, e.t.c) On The Server, 
                and Many More...


                                       By the_devsrealm_guy
                                       https://devsrealm.com/

" | boxes -d columns


#   This checks if the web server is installed
#   If No, It asks to be insttaled
web_server

#
#   If we got here, they must have supplied a filename.  Store it away
#
websitename=$1

#
#   Check if the filename represents a valid file.
#
if [ ! -e  $site_available/"$websitename" ]
then
    echo -e "\t\t\t\t$1 does not exist"

    #
    #   Ask if it should be created
    #
    if yes_no "Do you want to create it"
    then
        #
        #   Attempt to create it
        #
        > $site_available/"$websitename"

        #
        #   Check if that succeeeded, i.e does user has a permission to create a file
        #
        if [ ! -w $site_available/"$websitename" ]
        then
            echo "$1" could not be created, check your user permission
            exit 2
        fi
        #
        #   Otherwise we're OK
        #

    else
        #
        #   User doesn't want to create a file
        #
        exit 0
    fi
elif [ ! -w $site_available/"$websitename" ]    # it exists - check if it can be written to
then
    echo -e "\t\t\t\tCould not open $1 for writing, check your user permission"
    exit 2
fi

#
#   Loop forever - until the user decides to exit the program
#
while :
do
clear
    #
    #   Display the menu
    #  
    echo "

                           CommandLine ControlPanel Installation
                                      Main Menu

                    What do you wish to do?
                    1.) Install and Configure ClassicPress or WordPress
                    2.) Secure Site With Let's Encrypt SSL
                    3.) Create an SFTP User
                    4.) Manage DNS
                    5.) Install PHPMyAdmin
                    6.) Automate Backup and or Restore
                    7.) Exit

    " | boxes -d columns
    #
    #   Prompt for an answer
    #
    echo -e "\t\t\t\tAnswer (or 'q' to quit): \c?"
    read ans junk

    #
    #   Empty answers (pressing ENTER) cause the menu to redisplay,
    #   so, this goes back around the loop
    #   We only make it to the "continue" bit if the "test"
    #   program ("[") returned 0 (True)
    #
    [ "$ans" = "" ] && continue

    #
    #   Decide what to do
    #
    case $ans in
        1)     website_create
        ;;
        2)     website_secure
        ;;
        3)     sftp
        ;;
        4)     dns
        ;;
        5)     phpmyadmin
        ;;
        6)     backup
        ;;
        7)     quit 0
        ;;
        q*|Q*) quit 0
        ;;
        *)     echo -e "\t\t\t\tplease enter a number between 1 and 7";;
    esac
    #
    #   Pause to give the user a chance to see what's on the screen, this way, we won't lose some infos
    #
    pause
done
