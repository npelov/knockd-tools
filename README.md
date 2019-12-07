# knockd-tools
Tools to use with Judd Vinet's knockd

## ABOUT

This set of tools to use with [Judd Vinet's knockd](https://zeroflux.org/projects/knock) port-knocking server/client.
The same functionality can be accomplished with a small script, but port-close which will stay in ram for hours and
maybe days uses very little ram.

## INSTALL

To install the tools in /usr/local/sbin run:

    $ make
    $ sudo make install

If you want other directories, edit Makefile and change:

    INSTALL_DIR=/usr/local
    SBIN_DIR=$(INSTALL_DIR)/sbin

Or simply copy port-open to whatever dir you want and make a sym or hard link port-close pointing to port-open

## EXAMPLE

First you need to create **iptables** rules:

    iptables -N knock
    iptables -I INPUT -j knock 

For example configuration check `knockd-example.conf`

## TEST

To test your installation you can run:

    iptables -N knock
    port-open 8.8.8.8 80 20
    watch iptables -xvnL knock

You should see a rule accepting connections from 8.8.8.8 on port 80. After after 20 seconds the rule should disappear. 


    Chain knock (1 references)
        pkts      bytes target     prot opt in     out     source               destination
           0        0 ACCEPT     tcp  --  *      *       8.8.8.8              0.0.0.0/0           tcp dpt:80


## LINKS

 * [Judd Vinet's knockd](https://zeroflux.org/projects/knock)

