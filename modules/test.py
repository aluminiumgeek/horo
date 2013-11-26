# test.py (c) Mikhail Mezyakov <mihail265@gmail.com>
# Released under the GNU GPL v.3
# 
# Module sends "success" message to user on a channel

def horo(channel, user, args):
    """Send "success" message if everything is ok"""
    
    return u'PRIVMSG {channel} :{user}: success'.format(
        channel = channel,
        user = user
    )
