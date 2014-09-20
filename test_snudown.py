#!/usr/bin/python
# -*- coding: utf-8 -*-

import snudown
import unittest
import cStringIO as StringIO


cases = {
    '': '',
    'http://www.reddit.com':
        '<p><a href="http://www.reddit.com">http://www.reddit.com</a></p>\n',

    '[foo](http://en.wikipedia.org/wiki/Link_(film\))':
        '<p><a href="http://en.wikipedia.org/wiki/Link_(film)">foo</a></p>\n',

    '(http://tsfr.org)':
        '<p>(<a href="http://tsfr.org">http://tsfr.org</a>)</p>\n',

    '[A link with a /r/subreddit in it](/lol)':
        '<p><a href="/lol">A link with a /r/subreddit in it</a></p>\n',

    '[A link with a http://www.url.com in it](/lol)':
        '<p><a href="/lol">A link with a http://www.url.com in it</a></p>\n',

    '[Empty Link]()':
        '<p>[Empty Link]()</p>\n',

    'http://en.wikipedia.org/wiki/café_racer':
        '<p><a href="http://en.wikipedia.org/wiki/caf%C3%A9_racer">http://en.wikipedia.org/wiki/café_racer</a></p>\n',

    '#####################################################hi':
        '<h6>###############################################hi</h6>\n',

    '[foo](http://bar\nbar)':
        '<p><a href="http://bar%0Abar">foo</a></p>\n',

    '/r/test':
        '<p><a href="/r/test">/r/test</a></p>\n',

    'Words words /r/test words':
        '<p>Words words <a href="/r/test">/r/test</a> words</p>\n',

    '/r/':
        '<p>/r/</p>\n',

    r'escaped \/r/test':
        '<p>escaped /r/test</p>\n',

    'ampersands http://www.google.com?test&blah':
        '<p>ampersands <a href="http://www.google.com?test&amp;blah">http://www.google.com?test&amp;blah</a></p>\n',

    '[_regular_ link with nesting](/test)':
        '<p><a href="/test"><em>regular</em> link with nesting</a></p>\n',

    ' www.a.co?with&test':
        '<p><a href="http://www.a.co?with&amp;test">www.a.co?with&amp;test</a></p>\n',

    r'Normal^superscript':
        '<p>Normal<sup>superscript</sup></p>\n',

    r'Escape\^superscript':
        '<p>Escape^superscript</p>\n',

    r'~~normal strikethrough~~':
        '<p><del>normal strikethrough</del></p>\n',

    r'\~~escaped strikethrough~~':
        '<p>~~escaped strikethrough~~</p>\n',

    'anywhere\x03, you':
        '<p>anywhere, you</p>\n',

    '[Test](//test)':
        '<p><a href="//test">Test</a></p>\n',

    '[Test](//#test)':
        '<p><a href="//#test">Test</a></p>\n',

    '[Test](#test)':
        '<p><a href="#test">Test</a></p>\n',

    '[Test](git://github.com)':
        '<p><a href="git://github.com">Test</a></p>\n',

    '[Speculation](//?)':
        '<p><a href="//?">Speculation</a></p>\n',

    '/r/sr_with_underscores':
        '<p><a href="/r/sr_with_underscores">/r/sr_with_underscores</a></p>\n',

    '[Test](///#test)':
        '<p><a href="///#test">Test</a></p>\n',

    '/r/multireddit+test+yay':
        '<p><a href="/r/multireddit+test+yay">/r/multireddit+test+yay</a></p>\n',

    '<test>':
        '<p>&lt;test&gt;</p>\n',

    'words_with_underscores':
        '<p>words_with_underscores</p>\n',

    'words*with*asterisks':
        '<p>words<em>with</em>asterisks</p>\n',

    '~test':
        '<p>~test</p>\n',

    '/u/test':
        '<p><a href="/u/test">/u/test</a></p>\n',

    '/u/test/m/test test':
        '<p><a href="/u/test/m/test">/u/test/m/test</a> test</p>\n',

    '/U/nope':
        '<p>/U/nope</p>\n',

    '/r/test/m/test test':
        '<p><a href="/r/test/m/test">/r/test/m/test</a> test</p>\n',

    '/r/test/w/test test':
        '<p><a href="/r/test/w/test">/r/test/w/test</a> test</p>\n',

    '/r/test/comments/test test':
        '<p><a href="/r/test/comments/test">/r/test/comments/test</a> test</p>\n',

    '/u/test/commentscommentscommentscommentscommentscommentscomments/test test':
        '<p><a href="/u/test/commentscommentscommentscommentscommentscommentscomments/test">/u/test/commentscommentscommentscommentscommentscommentscomments/test</a> test</p>\n',


    '/r/test/commentscommentscommentscommentscommentscommentscomments/test test':
        '<p><a href="/r/test/commentscommentscommentscommentscommentscommentscomments/test">/r/test/commentscommentscommentscommentscommentscommentscomments/test</a> test</p>\n',

    'blah \\':
        '<p>blah \\</p>\n',

    '/r/whatever: fork':
        '<p><a href="/r/whatever">/r/whatever</a>: fork</p>\n',

    '/r/t:timereddit':
        '<p><a href="/r/t:timereddit">/r/t:timereddit</a></p>\n',

    '/r/reddit.com':
        '<p><a href="/r/reddit.com">/r/reddit.com</a></p>\n',

    '/r/not.cool':
        '<p><a href="/r/not">/r/not</a>.cool</p>\n',

    '/r/very+clever+multireddit+reddit.com+t:fork+yay':
        '<p><a href="/r/very+clever+multireddit+reddit.com+t:fork+yay">/r/very+clever+multireddit+reddit.com+t:fork+yay</a></p>\n',

    '/r/t:heatdeathoftheuniverse':
        '<p><a href="/r/t:heatdeathoftheuniverse">/r/t:heatdeathoftheuniverse</a></p>\n',

    '/r/all-minus-something':
        '<p><a href="/r/all-minus-something">/r/all-minus-something</a></p>\n',

    '/r/notall-minus':
        '<p><a href="/r/notall">/r/notall</a>-minus</p>\n',

    '/R/reddit.com':
        '<p>/R/reddit.com</p>\n',

    ('|' * 5) + '\n' + ('-|' * 5) + '\n|\n':
        '<table><thead>\n<tr>\n' + ('<th></th>\n' * 4) + '</tr>\n</thead><tbody>\n<tr>\n<td colspan="4" ></td>\n</tr>\n</tbody></table>\n',

    ('|' * 2) + '\n' + ('-|' * 2) + '\n|\n':
        '<table><thead>\n<tr>\n' + ('<th></th>\n' * 1) + '</tr>\n</thead><tbody>\n<tr>\n<td></td>\n</tr>\n</tbody></table>\n',

    ('|' * 65) + '\n' + ('-|' * 65) + '\n|\n':
        '<table><thead>\n<tr>\n' + ('<th></th>\n' * 64) + '</tr>\n</thead><tbody>\n<tr>\n<td colspan="64" ></td>\n</tr>\n</tbody></table>\n',

    ('|' * 66) + '\n' + ('-|' * 66) + '\n|\n':
        '<p>' + ('|' * 66) + '\n' + ('-|' * 66) + '\n|' + '</p>\n',
}

wiki_cases = {
    '<table scope="foo"bar>':
        '<p><table scope="foo"></p>\n',

    '<table scope="foo"bar colspan="2">':
        '<p><table scope="foo" colspan="2"></p>\n',

    '<table scope="foo" colspan="2"bar>':
        '<p><table scope="foo" colspan="2"></p>\n',

    '<table scope="foo">':
        '<p><table scope="foo"></p>\n',

    '<table scop="foo">':
        '<p><table></p>\n',

    '<table ff= scope="foo">':
        '<p><table scope="foo"></p>\n',

    '<table colspan= scope="foo">':
        '<p><table scope="foo"></p>\n',

    '<table scope=ff"foo">':
        '<p><table scope="foo"></p>\n',

    '<table scope="foo" test="test">':
        '<p><table scope="foo"></p>\n',

    '<table scope="foo" longervalue="testing test" scope="test">':
        '<p><table scope="foo" scope="test"></p>\n',

    '<table scope=`"foo">':
        '<p><table scope="foo"></p>\n',

    '<table scope="foo bar">':
        '<p><table scope="foo bar"></p>\n',

    '<table scope=\'foo colspan="foo">':
        '<p><table></p>\n',

    '<table scope=\'foo\' colspan="foo">':
        '<p><table scope="foo" colspan="foo"></p>\n',

    '<table scope=>':
        '<p><table></p>\n',

    '<table scope= colspan="test" scope=>':
        '<p><table colspan="test"></p>\n',

    '<table colspan="\'test">':
        '<p><table colspan="&#39;test"></p>\n',

    '<table scope="foo" colspan="2">':
        '<p><table scope="foo" colspan="2"></p>\n',

    '<table scope="foo" colspan="2" ff="test">':
        '<p><table scope="foo" colspan="2"></p>\n',

    '<table ff="test" scope="foo" colspan="2" colspan=>':
        '<p><table scope="foo" colspan="2"></p>\n',

    ' <table colspan=\'\'\' a="" \' scope="foo">':
        '<p><table scope="foo"></p>\n',
}

class SnudownTestCase(unittest.TestCase):
    def __init__(self, renderer=snudown.RENDERER_USERTEXT):
        self.renderer = renderer
        unittest.TestCase.__init__(self)

    def runTest(self):
        output = snudown.markdown(self.input, renderer=self.renderer)

        for i, (a, b) in enumerate(zip(repr(self.expected_output),
                                       repr(output))):
            if a != b:
                io = StringIO.StringIO()
                print >> io, "TEST FAILED:"
                print >> io, "       input: %s" % repr(self.input)
                print >> io, "    expected: %s" % repr(self.expected_output)
                print >> io, "      actual: %s" % repr(output)
                print >> io, "              %s" % (' ' * i + '^')
                self.fail(io.getvalue())



def test_snudown():
    suite = unittest.TestSuite()

    for input, expected_output in wiki_cases.iteritems():
        case = SnudownTestCase(renderer=snudown.RENDERER_WIKI)
        case.input = input
        case.expected_output = expected_output
        suite.addTest(case)

    for input, expected_output in cases.iteritems():
        case = SnudownTestCase()
        case.input = input
        case.expected_output = expected_output
        suite.addTest(case)

    return suite
