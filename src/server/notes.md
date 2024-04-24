# Request types from client

- r (register client):
  - e (error in client registration, probably becuse client with same username already exists)
  - r (client sucessfully registered)
- l (login client):
  - l (client sucessfully logined)
  - w (failed to log in client - wrong password)
  - f (failed to login client - error, username probably does not exist)
- m (send message):
  - s (message send sucussfully)
  - c (failed to send message)
- g (get messages):
  - g (messages get)
- n (get new messages):
  - n (new messages get)
- c (close connection):
  - c (closing network connection)

if server does not recognize client request, then server return "q"
