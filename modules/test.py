# test.py (c) Mikhail Mezyakov <mihail265@gmail.com>
# Released under the GNU GPL v.3
# 
# Module sends "SUCCESS" message to user on a channel

def horo(channel, user, args):
    return u'PRIVMSG {channel} :{user}: success'.format(
        channel = channel,
        user = user
    )
