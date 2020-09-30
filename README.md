# Automate Multiple Classic and WordPress Sites + [Caching and SSL]
If you are tired of using control panels (I am because they are mostly not secure), and want to do everything on the server level, I wrote a menu-driven bash program that handles the installation of ClassicPress or WordPress using Nginx (+ Caching), PHP and mariadb.

It also provides the ability to secure your website using Let's Encrypt, and Certbot for the auto-config. I made this optional because am I just want to do some local development, so, I won't need an SSL in that regard.

As of the time of writing this, it took only 1min.34s (proof in the video below) to configure a website on a fresh server instance, it shouldn't take more than 30s to add other sites.

►Key Requirements

- Operating system: Ubuntu LTS
- Memory: At least 512MB GB RAM 
- The installation must be performed on a clean system, free of installed/configured software, especially anything related to web servers.
- The installation must be done as a root user, it won't even allow you to pass through even if you want to.
- If you want to use the Let's Encrypt Feature, make sure you have an A record for the website

## How To Install The Program

The installation is easy:

### Connect to your server as root via SSH

> ssh root@yourserver

### Download The Program, and Change Your Diretory To The Program Directory 

> git clone https://github.com/devsrealm/commandline_controlpanel.git && cd commandline_controlpanel

### Give It an Executable Permission

> chmod +x cp

### Run The Program, and Configure Away

> ./cp yourwebsite.com

*Note: Make Sure You Always Add a Top Level Domain, i.e .com, .net, etc*

First Posted Here: [Automate Multiple ClassicPress & Wordpress Installation](https://devsrealm.com/bash/automate-classicpress-wordpress-installation/)

Reach me at: [Devsrealm](https://devsrealm.com/)