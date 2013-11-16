# test_event.py (c) Mikhail Mezyakov <mihail265@gmail.com>
# Released under the GNU GPL v.3
#
# Events test. Module returns a message contains triggered event

EVENTS = ['test', 'say']

def horo(channel, user, msg):
    print msg
    for event in EVENTS:
        if msg.startswith(event):
            return "PRIVMSG {channel} :{user}: event triggered by '{text}' text".format(
                channel = channel,
                user = user,
                text = event
            )
