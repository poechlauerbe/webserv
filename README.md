
# Webserv

HTTP web server written in C/C++98. Can serve fully static websites. Pass .config as argument or it will take default webserv.conf. Supports multiple server blocks and location blocks with different server_names and ports. 

Supports
 -
cgi-bin (eg: .py, .sh)
auto-index
error_page
client-max-body-size
allowed_methods (GET, POST, DELETE)
redirection to location
redirection to URL
and more . . . .


## How to run ?

Simply copy commands

```bash
  git clone https://github.com/poechlauerbe/webserv.git
  cd webserv
  make
  ./webserv webserv.conf
```

## Contributors

- [Benjamin Pöchlauer](https://github.com/poechlauerbe)

- [Thorben Benz](https://github.com/BenzThor)

- [Sumon Md Ohiduzzaman](https://github.com/sumon-ohid)

## Screenshots

<!-- ![Webserv Screenshot](https://via.placeholder.com/468x300?text=App+Screenshot+Here) -->
<img width="1459" alt="webservHome" src="https://github.com/user-attachments/assets/75f3b101-236f-4f0a-9b78-bd73aa6eaa6d">

## 🛠 Skills Achieved
C/C++, HTML, CSS, JAVASCRIPT, NGINX, PYTHON etc.

## License

Feel free to use and distribute.

[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](https://choosealicense.com/licenses/mit/)


# Test Cases

## To test post with curl

```
curl -F "file=@testUpload.txt" http://localhost:8000/
```

## To check a port process

```
lsof -t -i :PORT
```

## Siege Stress Test

```
siege -c 250 -r 1000 http://ip:port
```
