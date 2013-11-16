# youtube_info.py (c) Mikhail Mezyakov <mihail265@gmail.com>
# Released under the GNU GPL v.3
#
# This is a core-module!
# Required Google Data Python Client Library (http://code.google.com/p/gdata-python-client/)
# Written in 2010, sorry for that code

import gdata.youtube
import gdata.youtube.service
import sys

yt_service = gdata.youtube.service.YouTubeService()
yt_service.developer_key = 'AI39si6O9KHDa-ejp--h9cPqziQQbqPcdI5hKRffITHJ60OAnA9tE0eR2kkPtq5QZDdM0gyvnebbGT3LW-AYuuw0H2xC2-DJ3w'

yt_service.email = 'YOU@gmail.com'
yt_service.password = 'YOUR_PASSWORD'
yt_service.source = 'k-modules'

#yt_service.ProgrammaticLogin()

# Video id from the link
video_id = sys.argv[len(sys.argv)-1]
if video_id.find('&') != -1: 
    video_id = video_id[video_id.find('v=')+2:video_id.find('&')]
else: 
    video_id = video_id[video_id.find('v=')+2:]

entry = yt_service.GetYouTubeVideoEntry(video_id=video_id)

# Number of comments and latest comment
comment_feed = yt_service.GetYouTubeVideoCommentFeed(video_id=video_id)
comments = len(comment_feed.entry)

if comment_feed.entry:
    l_comment = comment_feed.entry[0].ToString()
    l_comment = l_comment[l_comment.find('<ns0:content type="text">')+25:l_comment.find("</ns0:content>")]
else: 
    l_comment = '<none>'

if entry.statistics: 
    views = entry.statistics.view_count
else: 
    views = '0'

if entry.rating:
    result = views + ' views; rating: ' + entry.rating.average[:3] + '; ' + str(comments) + ' comments; added ' + entry.published.text[:10]
else:
    result = views + ' views; ' + str(comments) + ' comments; added ' + entry.published.text[:10]

if len(l_comment) > 105: 
    result += '. Latest comment: ' + l_comment[:105] + '...'
else: 
    result += '. Latest comment: ' + l_comment

print result