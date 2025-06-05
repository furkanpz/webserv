<a name="readme-top"></a>

<h1 align="center">Webserv</h1>
<!-- ABOUT THE PROJECT -->

<p align="center">
<img width="200" src="https://github.com/oph-design/webserv/assets/115570424/302d6a9b-c5ce-48e9-9a11-c6fa005613bd"/>
</p>

Webserv is a project from the 42 curriculum, aiming to build a webserver based on NGINX. The implementation follows the latest guidelines from the RFCs for the HTML protocol and CGI protocol. The server is completely build in C++ and uses poll as the multiplexer to manage the socket connections. The implementation covers the following features:
* Handling of `GET`, `POST` and `DELETE` requests
* Serving of every common Filetype
* Nginx-like configuration with config files
* Common Gateway Interface for every requests type
* Autoindexing

