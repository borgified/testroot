#!/usr/bin/env python
# vim: smartindent tabstop=4 shiftwidth=4 expandtab number
#
# This file is part of the Assimilation Project.
#
# Author: Alan Robertson <alanr@unix.sh>
# Copyright (C) 2014 - Assimilation Systems Limited
#
# Free support is available from the Assimilation Project community - http://assimproj.org
# Paid support is available from Assimilation Systems Limited - http://assimilationsystems.com
#
# The Assimilation software is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Assimilation software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with the Assimilation Project software.  If not, see http://www.gnu.org/licenses/
#
#
'''
This file defines classes which perform individual system tests.
'''
import sys, time
from logwatcher import LogWatcher
from querytest import QueryTest
from docker import TestSystem
sys.path.append('..')
import graphnodes as GN
class AssimSysTest(object):
    '''AssimSysTest is an abstract base class for all our system-level tests.
    '''
    SUCCESS = 1
    FAIL    = 2
    SKIPPED = 3

    testset = []
    stats = {}

    @staticmethod
    def register(cls):
        AssimSysTest.testset.append(cls)
        AssimSysTest.stats[cls.__name__] = {
            AssimSysTest.SUCCESS:0, AssimSysTest.FAIL: 0, AssimSysTest.SKIPPED:0
        }
        return cls

    def __init__(self, store, logfilename, testenviron, debug=False):
        self.store = store
        self.logfilename = logfilename
        self.testenviron = testenviron
        self.debug = debug

    def _record(self, result):
        AssimSysTest.stats[self.__class__.__name__][result] += 1
        #print >> sys.stderr, '_RECORD RETURNING', result
        return result

    def checkresults(self, watcher, timeout, querystring, validator
        ,   nano, service=None, allregexes=False, debug=False, minrows=1, maxrows=1):

        query = QueryTest(self.store, querystring, GN.nodeconstructor, debug=debug)
        if allregexes:
            match = watcher.lookforall(timeout=timeout)
        else:
            match = watcher.look(timeout=timeout)
        if debug:
            print >> sys.stderr, ('DEBUG: Match returned %s' % match)
        if match is None:
            print ('ERROR: Test %s timed out waiting for %s [timeout:%s]'
            %   (self.__class__.__name__, str(watcher.regexes), timeout))
            return self._record(AssimSysTest.FAIL)
        if debug:
            print('DEBUG: Test %s found regex %s [timeout:%s]'
            %   (self.__class__.__name__, str(watcher.regexes)))
        if query.check((nano, self.testenviron.cma, service), validator
            ,       minrows=minrows, maxrows=maxrows):
            if debug:
                print('DEBUG: Test %s passed query %s'
                %   (self.__class__.__name__, querystring))
            return self._record(AssimSysTest.SUCCESS)

        print >> sys.stderr, ('DEBUG: query.check() FAILED')
        print('ERROR: Test %s failed query %s' %  (self.__class__.__name__, querystring))
        return self._record(AssimSysTest.FAIL)
        
    def run(self):
        raise NotImplementedError('AssimSysTest.run is an abstract method')

@AssimSysTest.register
class StopNanoprobe(AssimSysTest):
    def run(self, nano=None, debug=None, timeout=30):
        if debug is None:
            debug = self.debug
        if nano is None:
            nano = self.testenviron.select_nano_service()[0]
        if (nano is None or nano.status != TestSystem.RUNNING or 
            SystemTestEnvironment.NANOSERVICE not in nano.runningservices):
            return self._record(AssimSysTest.SKIPPED)
        regex = (r'%s cma INFO: System %s at \[::ffff:%s]:1984 reports graceful shutdown'
        %   (self.testenviron.cma.hostname, nano.hostname, nano.ipaddr))
        watch = LogWatcher(self.logfilename, (regex,), timeout=timeout, debug=debug)
        watch.setwatch()
        qstr =  (   '''START drone=node:Drone('*:*') '''
                     '''WHERE drone.designation = "{0.hostname}" and drone.status = "dead" '''
                     '''and drone.reason = "HBSHUTDOWN"       RETURN drone''')
        nano.stopservice(SystemTestEnvironment.NANOSERVICE)
        return self._record(AssimSysTest.SUCCESS
                if self.checkresults(watch, timeout, qstr, None, nano) == AssimSysTest.SUCCESS
                else AssimSysTest.FAIL)

@AssimSysTest.register
class StartNanoprobe(AssimSysTest):
    def run(self, nano=None, debug=None, timeout=30):
        if debug is None:
            debug = self.debug
        if nano is None:
            nano = self.testenviron.select_nano_noservice()[0]
        if (nano is None or nano.status != TestSystem.RUNNING
            or  SystemTestEnvironment.NANOSERVICE in nano.runningservices):
            return self._record(AssimSysTest.SKIPPED)

        regex = (' %s cma INFO: Stored packages JSON data from %s '
        %           (self.testenviron.cma.hostname, nano.hostname))
        regex = (r' %s cma INFO: Drone %s registered from address \[::ffff:%s]'
        %           (self.testenviron.cma.hostname, nano.hostname, nano.ipaddr))
        watch = LogWatcher(self.logfilename, (regex,), timeout=timeout, debug=debug)
        watch.setwatch()
        qstr = (    '''START drone=node:Drone('*:*') '''
                     '''WHERE drone.designation = "{0.hostname}" and drone.status = "up" '''
                     '''RETURN drone''')
        nano.startservice(SystemTestEnvironment.NANOSERVICE)
        return self._record(AssimSysTest.SUCCESS
                if self.checkresults(watch, timeout, qstr, None, nano) == AssimSysTest.SUCCESS
                else AssimSysTest.FAIL)

@AssimSysTest.register
class FlipNanoprobe(AssimSysTest):
    def run(self, nano=None, debug=None, timeout=30):
        if debug is None:
            debug = self.debug
        if nano is None:
            nano = self.testenviron.select_nanoprobe()[0]
        if nano is None:
            return self._record(AssimSysTest.SKIPPED)
        if SystemTestEnvironment.NANOSERVICE in nano.runningservices:
            return self._record(
                StopNanoprobe(self.store
                ,   self.logfilename, self.testenviron).run(nano, debug=self.debug))
        return self._record(
            StartNanoprobe(self.store, self.logfilename
            ,   self.testenviron).run(nano, debug=self.debug))

@AssimSysTest.register
class RestartNanoprobe(AssimSysTest):
    def __init__(self, store, logfilename, testenviron, debug=False, delay=0):
        AssimSysTest.__init__(self, store, logfilename, testenviron, debug)
        self.delay = delay

    def run(self, nano=None, debug=None, timeout=30):
        if debug is None:
            debug = self.debug
        if nano is None:
            nano = self.testenviron.select_nano_service()[0]
        if nano is None:
            return self._record(AssimSysTest.SKIPPED)
        rc = StopNanoprobe(self.store
            ,   self.logfilename, self.testenviron).run(nano, debug=self.debug)
        if rc != AssimSysTest.SUCCESS:
            return self._record(rc)
        if self.delay > 0:
            time.sleep(self.delay)
        return self._record(StartNanoprobe(self.store
        ,   self.logfilename, self.testenviron).run(nano, debug=self.debug))

# A little test code...
if __name__ == "__main__":
    import os
    from docker import SystemTestEnvironment
    from store import Store
    from cmainit import CMAinit
    from py2neo import neo4j
    def testmain(logname, maxdrones=3, debug=False):
        'A simple test main program'
        regexes = []
        #pylint says: [W0612:testmain] Unused variable 'j'
        #pylint: disable=W0612
        for j in range(0,maxdrones+1):
            regexes.append('Stored packages JSON data from *([^ ]*) ')
        logwatch = LogWatcher(logname, regexes, timeout=90, returnonlymatch=True)
        logwatch.setwatch()
        sysenv = SystemTestEnvironment(maxdrones)
        url = ('http://%s:%d/db/data/' % (sysenv.cma.ipaddr, 7474))
        CMAinit(None)
        store = Store(neo4j.GraphDatabaseService(url), readonly=True)
        for classname in GN.GraphNode.classmap:
            GN.GraphNode.initclasstypeobj(store, classname)
        if debug:
            print >> sys.stderr, 'WATCH RESULTS:', logwatch.lookforall()
        tq = QueryTest(store
        ,   "START drone=node:Drone('*:*') RETURN drone"
        ,   GN.nodeconstructor, debug=debug)
        if tq.check([None,], minrows=maxdrones+1, maxrows=maxdrones+1):
            print 'WOOT! Systems passed query check after initial startup!'
        else:
            print 'Systems FAILED initial startup query check'
            print 'Do you have a second CMA running??'

        assert FlipNanoprobe(store, logname, sysenv, debug=debug).run() == AssimSysTest.SUCCESS
        assert StopNanoprobe(store, logname, sysenv, debug=debug).run() == AssimSysTest.SUCCESS
        assert StartNanoprobe(store, logname, sysenv, debug=debug).run() == AssimSysTest.SUCCESS
        assert FlipNanoprobe(store, logname, sysenv, debug=debug).run() == AssimSysTest.SUCCESS
        assert RestartNanoprobe(store, logname, sysenv, debug=debug, delay=30).run() == AssimSysTest.SUCCESS
        print >> sys.stderr, 'All tests were successful!'

    if os.access('/var/log/syslog', os.R_OK):
        sys.exit(testmain('/var/log/syslog', debug=False))
    elif os.access('/var/log/messages', os.R_OK):
        sys.exit(testmain('/var/log/messages', debug=False))
