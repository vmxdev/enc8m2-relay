# enc8m2-relay

Recently I got few video encoders ENC 8m2

I had to make it work with zoneminder

It turned out that it's not easy

The encoder sends all streams from different cameras to one UDP-port. Flows differ only by source port. Ffmpeg can not (or I can't tell to ffmpeg how) to separate such flows

It seems that this can be done with iptables (rules REDIRECT and DNAT), but on a machine with a single network interface it wasn't an easy task. Redirect to loopback don't work, although I have tried all (well, almost) the recipes from google. I did by creating a virtual interface and redirecting to it, but there is another problem

The encoder adds to each packet its headers (16 bytes in my case). It looks like timestamps and some more information (possibly about cameras), causing artifacts on video 

To cut the headers and redirect different mpeg-ts streams to different ports, I wrote a simple utility, UDP-relay.                         

Launch as:

```sh
$ ./enc8m2-relay 60001 192.168.1.0 16
```

where `60001` is UDP-port which receives a stream

`192.168.1.0` - address, which will be considered as base 

If you have two encoder with addresses 192.168.1.22 and 192.168.1.33, relay redirect flows in this way: stream from first camera of first encoder will be redirected to 127.0.0.1:60220, from second camera to 127.0.0.1:60221 and so on. Streams from second encoder will be redirected to 127.0.0.1:60330, 127.0.0.1:60331, 127.0.0.1:60332 etc.

`16` - number of header bytes that need to be cut in each packet
