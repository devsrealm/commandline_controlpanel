#!/bin/bash
############################################
#                                          #
#   Scirpt That Automate The Installation  #
#       Of ClassicPress and WordPress      #
#         Using Nginx Web Server           #
#                                          #
#    Author: The_Devsrealm_Guy             #
#   Website: https://devsrealm.com         #
#   Last Edited: September 2020            #
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
        #   Display the string passed in in $1, followed by "(Y/N)?"
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
 if command -v nginx 2>> ${logfile} >/dev/null
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
          sudo apt-get update 2>> ${logfile} >/dev/null &
          sudo apt-get -y install nginx 2>> ${logfile} >/dev/null &

          # Spinning, While the program installs
          spinner  

          #   Recheck if nginx is installed
          #   Pause to give the user a chance to see what's on the screen
          #
          if command -v nginx 2>> ${logfile} >/dev/null
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
    certbot --nginx -d "$websitename" -d "www.$websitename" -m $email --agree-tos --redirect --hsts --staple-ocsp 2>> ${logfile} >/dev/null &
    errorchecker_certbot $?
    echo
    echo -e "\t\t\t\tDone\n"

}

#
#   website_secure()
#
#   Secure website using Let’s Encrypt SSL
#

      
website_secure()
{
  if command -v certbot 2>> ${logfile} >/dev/null
    then
      echo
      echo -e "\t\t\t\tcertbot is available\n" "\xE2\x9C\x94\n"

      if yes_no "Do you want to secure another website"
       then
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
        sudo apt-get update 2>> ${logfile} >/dev/null &
        sudo apt-get -y install python-certbot-nginx 2>> ${logfile} >/dev/null &
        # Spinning, While the program installs
        spinner

        #
        #   If we got here, then it means that we are done with installing certbot
        #   Call The install_certbot function away
        #
        install_certbot

       # reload nginx
        sudo systemctl enable nginx 2>> ${logfile} >/dev/null &
        sudo systemctl reload nginx 2>> ${logfile} >/dev/null &
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
    echo -e "\t\t\t\tUsage: `basename $script` $*\n" 1>&2

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
        exit $code           #  exit using the supplied code.
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
    site_available=/etc/nginx/sites-available
    site_enabled=/etc/nginx/sites-enabled
    [ -d $site_available ] || sudo mkdir -p $site_available 
    [ -d $site_enabled ] || sudo mkdir -p $site_enabled

    # 
    #   mktemp will create the file or exit with a non-zero exit status, 
    #   this way, you can ensures that the script will exit if mktemp is unable to create the file.
    #
    #   Note: The inclusion of /etc/nginx/sites-enabled/*; is no longer need, I'll leave this for reference
    #
    #   This adds the include /etc/nginx/sites-enabled/*; in the nginx config file if it isn't alread there
    #   We also removed any duplicate of /etc/nginx/sites-enabled/*
    #
    #   Note: I added two tabs to make the format of the nginx config consistent
    #    \t is one tab \\t is two tab, if you want three tab, you do \\t\t, yh, sed is crazy
    #

    TMPFILE=`mktemp /tmp/nginx.conf.XXXXXXXXXX` || exit 1
    cat ngx_conf_with_caching | sed '/conf.d/a  \\tinclude /etc/nginx/sites-enabled/*;' | awk '!(/sites-enabled/ && seen[$0]++)' > $TMPFILE
    sudo cp -f $TMPFILE /etc/nginx/nginx.conf # move the temp to nginx.conf
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



    TMPFILE=`mktemp /tmp/default.nginx.XXXXXXXX` || exit 1

    cat ngx_serverblock | sudo sed -e "s/domain.tld/$websitename www.$websitename/g" -e "s/\/var\/www\/wordpress/\/var\/www\/$websitename/" > $TMPFILE

    sudo cp -f $TMPFILE $site_available/$websitename

    # remove the tempfile
    rm "$TMPFILE"
    #

    #
    #   Create a directory for the root directory if it doesn't already exist
    #

    if [ ! -d /var/www/$websitename ];then
    sudo mkdir /var/www/$websitename
    fi

    #   
    #   Nginx comes with a default server block enabled (virtual host), let’s remove the symlink, we then add the new one
    #

    if [ -f $site_enabled/default ];then

      sudo unlink $site_enabled/default 2>> ${logfile} >/dev/null &

    fi


    errorchecker $?

    #   
    #   Check if symbolik link exist already
    #


    if [ ! -f $site_enabled/$websitename ];then

          sudo ln -s $site_available/$websitename /etc/nginx/sites-enabled/ 2>> ${logfile} >/dev/null &

    fi

    # reload nginx
    sudo systemctl enable nginx  2>> ${logfile} >/dev/null &
    sudo systemctl reload nginx 2>> ${logfile} >/dev/null &

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

        if [ $mysqlpass != $mysqlpass2 ]; then
            echo -e "\t\t\t\tPasswords do not match, Please Try again"
        else
            echo -e "\t\t\t\tPasswords Matches, Moving On..." 
            echo
            break
        fi

    done # Endwhile loop

    TMPFILE=`mktemp /tmp/mysql_secure_installation.XXXXXXXXXX` || exit 1
    cat mysql_secure_installation.sql | sed -e "s/123456789/$mysqlpass/" > $TMPFILE
    sudo cp -f $TMPFILE mysql_secure_installation.sql # move the temp to mysql_secure_installation.sql
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
  
if command -v mariadb 2>> ${logfile} &>/dev/null && command -v php 2>> ${logfile} &>/dev/null
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
          php7.2-curl 2>> ${logfile} >/dev/null &

          # Spinning, While the program installs
          spinner  

            pause_webserver PHP, Mariadb 
            echo
            echo -e "\t\t\t\tLet's Secure Your Mariadb Server"
            echo
            mysql_secure_installation
            install_cp_wp

          sudo systemctl start  mariadb 2>> ${logfile} >/dev/null &

          sudo systemctl enable  mariadb 2>> ${logfile} >/dev/null &
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

                    echo -e "\t\t\t\tEnter ClassicPress or wordpress Password For $CpDBUser: \c"
                    read -s CpDBPass # Adding the -s option to read hides the input from being displayed on the screen.
                    echo -e "\t\tRepeat Password: \c"
                    read -s CpDBPass2 # Adding the -s option to read hides the input from being displayed on the screen.
                                # 
                      #   Checking if both passwords match
                      #

                      if [ $CpDBPass != $CpDBPass2 ]; then
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
            # Downloading latest Wordpress tarall and extraction
            #

           TMPDIR=`mktemp -d /tmp/cp_wp.XXXXXXXXXX` || exit 1
            echo
            echo -e "\t\t\t\tOh! Wait, Is The Details You Supplied for ClassicPress or WordPress?\n"
            while :
            do
                #
                #   Display the ClassicPress or WordPress Decision Menu
                #
              echo "
                              1.) ClassicPress
                              2.) Wordpress
                              3.) Exit

              " | boxes -d columns

          #  echo -e "\tType cp For ClassicPress or wp for WordPress: \c"
          echo -e "\t\t\t\tChoose 1 For ClassicPress or 2 for WordPress: \c"
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
                echo -e "\t\t\t\tWe Proceed with  ClassicPress\n"
                echo -e "\t\t\t\tDownloading Latest Classicpress To a Temp Directory"
        
                wget https://www.classicpress.net/latest.tar.gz -O $TMPDIR/cplatest.tar.gz 2>> ${logfile} &>/dev/null

                spinner
                #
                #   Extract the file, and extract it into a folder
                #

                mkdir -p $TMPDIR/classicpress && tar -zxf  $TMPDIR/cplatest.tar.gz -C $TMPDIR/classicpress --strip-components 1 2>> ${logfile} &>/dev/null

                errorchecker $?

                cp -f $TMPDIR/classicpress/wp-config-sample.php $TMPDIR/classicpress/wp-config.php &>/dev/null

                errorchecker $?

                cp -a $TMPDIR/classicpress/. /var/www/$websitename &>/dev/null

                errorchecker $?

                sudo rm -R $TMPDIR

                sudo rm -f /var/www/html/index.nginx-debian.html &>/dev/null

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

                chown -R www-data:www-data /var/www/$websitename

                #
                #   Chnage permission of all directroy and file under websitename
                #

                find /var/www/$websitename -type d -exec chmod 755 {} \;

                find /var/www/$websitename -type f -exec chmod 644 {} \;

                #
                #   Chnage permission of wp-config
                #

               chmod 660  /var/www/$websitename/wp-config.php

                #
                #   Allow ClassicPress To Manage Wp-content
                #

                find /var/www/$websitename/wp-content -type d -exec chmod 775 {} \;
                find /var/www/$websitename/wp-content -type f -exec chmod 664 {} \;

                #
                #   Writing ClassicPress config file with collected config data
                #

                echo -e "\t\t\t\tWriting ClassicPress config file with collected config data...\n"

                sed -i "s/database_name_here/$CpDBName/" /var/www/$websitename/wp-config.php

                sed -i "s/username_here/$CpDBUser/" /var/www/$websitename/wp-config.php

                sed -i "s/password_here/$CpDBPass/" /var/www/$websitename/wp-config.php

                progress_bar
                # reload nginx
                sudo systemctl reload nginx 2>> ${logfile} >/dev/null &
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
                wget https://wordpress.org/latest.tar.gz -O $TMPDIR/wplatest.tar.gz 2>> ${logfile} &>/dev/null

                spinner
                #

                #
                #   Extract the file, and extract it into a folder
                #
                mkdir -p $TMPDIR/wordpress && tar -zxf  $TMPDIR/latest.tar.gz -C $TMPDIR/wordpress --strip-components 1 2>> ${logfile} &>/dev/null

                errorchecker $?

                cp -f $TMPDIR/wordpress/wp-config-sample.php $TMPDIR/wordpress/wp-config.php 2>> ${logfile} &>/dev/null

                errorchecker $?

                cp -a $TMPDIR/wordpress/. /var/www/$websitename 2>> ${logfile} &>/dev/null

                errorchecker $?

                sudo rm -R $TMPDIR

                sudo rm -f /var/www/html/index.nginx-debian.html 2>> ${logfile} &>/dev/null

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

                chown -R www-data:www-data /var/www/$websitename

                #
                #   Chnage permission of all directroy and file under websitename
                #

                find /var/www/$websitename -type d -exec chmod 755 {} \;

                find /var/www/$websitename -type f -exec chmod 644 {} \;

                #
                #   Chnage permission of wp-config
                #

               chmod 660  /var/www/$websitename/wp-config.php

                #
                #   Allow WordPress To Manage Wp-content
                #

                find /var/www/$websitename/wp-content -type d -exec chmod 775 {} \;
                find /var/www/$websitename/wp-content -type f -exec chmod 664 {} \;

                #
                #   Writing Wordpress config file with collected config data
                #
                echo
                echo -e "\t\t\t\tWriting Wordpress config file with collected config data...\n"

 
                sed -i "s/database_name_here/$CpDBName/" /var/www/$websitename/wp-config.php

                sed -i "s/username_here/$CpDBUser/" /var/www/$websitename/wp-config.php

                sed -i "s/password_here/$CpDBPass/" /var/www/$websitename/wp-config.php

                # reload nginx
                sudo systemctl reload nginx 2>> ${logfile} >/dev/null &

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
                return 0
               ;;
            *)
                echo
                echo -e "\tplease enter a number between 1 and 2"
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
id $sftp_user &> /dev/null
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

              if [ $pass1 != $pass2 ]; then
                  echo -e "\t\t\t\tPasswords do not match, Please Try again"
              else
                  echo -e "\t\t\t\tPasswords Matches, Moving On..." 
                  break
              fi
          done # Endwhile loop

          echo "$sftp_user:$pass2" | chpasswd -c SHA512 #Encrypt Password using SHA512  
          echo -e "\t\t\tPassword Changed"
          return 0
      fi

else

  if [ ! -d $chroot_dir/$sftp_user ];then
    echo -e "\t\t\t\t$sftp_user directory and user does not exist yet...creating"

    mkdir -p $chroot_dir/$sftp_user/
    mkdir -p $chroot_dir/$sftp_user/$websitename
    echo -e "\t\t\t\t$sftp_user created"
  fi  

    #
    #   Adding the new user with the home directory with no ability to shell login  
    #   The user won't be able to SSH into the server, but can only access through SFTP for transferring files
    #

    useradd -d $chroot_dir/$sftp_user/ -s /usr/sbin/nologin -G $SFTP_ONLY_GROUP $sftp_user

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

        if [ $pass1 != $pass2 ]; then
            echo -e "\t\t\t\tPasswords do not match, Please Try again"
        else
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
    chmod 755 $chroot_dir/$sftp_user/
    chown root:root $chroot_dir # chowning the chroot_directory

    #   Setting the permissions of the mount directory

    chown $sftp_user:$SFTP_ONLY_GROUP $chroot_dir/$sftp_user/$websitename/
    chmod 700 $chroot_dir/$sftp_user/$websitename/

    #
    #   The below forces any new files or directories created by this user to have a group matching the parent directory
    #

    find /var/www/$websitename/ -type d -exec chmod g+s {} \;

    #
    #   Now we mount a specific directory to the users chrooted home directory.
    #   The below command mount the directory stated in /etc/fstab to the directory we have just specified 
    #   ($chroot_dir/$sftp_user/$websitename/)
    #

      mount -o bind /var/www/$websitename $chroot_dir/$sftp_user/$websitename

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

     return 0

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
[ $# == 1 ] || usage $0 websitename.com         # exits the program
echo


logfile=errorlog.txt

sudo apt-get -y install boxes 2>> ${logfile} &>/dev/null &
spinner

echo "

                Program For Automating The Installation of
                ClassicPress or Wordpress Using Nginx, Php
                                & Mariadb


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
if [[ ! -f  $site_available/$websitename ]]
then
    echo -e $1 "\t\t\t\tdoes not exist"

    #
    #   Ask if it should be created
    #
    if yes_no "Do you want to create it"
    then
        #
        #   Attempt to create it
        #
        > $site_available/$websitename

        #
        #   Check if that succeeeded, i.e does user has a permission to create a file
        #
        if [ ! -w $site_available/$websitename ]
        then
            echo $1 could not be created, check your user permission
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
elif [ ! -w $site_available/$websitename ]    # it exists - check if it can be written to
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

                           Classicpress/WordPress Installation
                                      Main Menu

                    What do you wish to do?
                    1.) Install and Configure ClassicPress or WordPress
                    2.) Secure Site With Let's Encrypt SSL
                    3.) Create an SFTP User
                    4.) Exit

    " | boxes -d columns
    #
    #   Prompt for an answer
    #
    echo -e "\t\t\t\tAnswer (or 'q' to quit): \c?"
    read ans junk

    #
    #   Empty answers (pressing ENTER) cause the menu to redisplay,
    #   so .... back around the loop
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
        4)     quit 0
        ;;
        q*|Q*) quit 0
        ;;
        *)     echo -e "\t\t\t\tplease enter a number between 1 and 3";;
    esac
    #
    #   Pause to give the user a chance to see what's on the screen, this way, we won't lose some infos
    #
    pause
done