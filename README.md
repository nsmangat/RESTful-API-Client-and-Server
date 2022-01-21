RESTful API using own client and server created in C for an assignment in CSCN72020 - Computer Networks
Given base of client and server and set up of sockets
Task was to recreate a RESTful API in C without using external libraries, only the initial Network Library 

For server implement:
  2 URIs: single post ie /posts/# and whole collection ie /posts
  GET single, GET collection, PUT, POST, DELETE
  Add attributes to post including post title, author and topic
  Load posting collection from disk and save it to file on shutdown of program

For client implement:
  Console program to allow user to use all the above methods implemented in the server

Included bonus feature of being to retrieve a set of posts based on filtering ie Get all posts with this specific author
