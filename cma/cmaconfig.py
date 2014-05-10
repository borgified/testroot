#!/usr/bin/python
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
'''
This file implements things related to Configuration files for the CMA.
Not quite sure what all it will do, but hey, this comment is slightly better than nothing.
'''
from AssimCclasses import pyConfigContext, pyNetAddr, pySignFrame, pyCompressFrame
from types import ClassType
class ConfigFile:
    '''
    This class implements configuration file management, including validation
    and default values for parameters.
    '''
    # A template is a pattern for how to validate a dict-like object
    # like those that come from pyConfigContexts -- which in turn model JSON
    default_template = {
        'OUI':                  {str: str}, # Addendum for locally-known OUI mappings
        'optional_modules':     [str],      # List of optional modules to be included
        'contrib_modules':      [str],      # List of contrib modules to be included
        'cmaport':              int,        # CMA listening port
        'cmainit':              pyNetAddr,  # Initial contact address for the CMA
        'cmaaddr':              pyNetAddr,  # CMA's base address...
        'cmadisc':              pyNetAddr,  # Address to send discovery information
        'cmafail':              pyNetAddr,  # Address to send failure reports
        'outsig':               pySignFrame,# Packet signature frame
        'compress':             pyCompressFrame,# Packet compression frame
        'compression_method':   {"zlib"},   # Packet compression method
        'compression_threshold':int,        # Threshold for when to start compressing
        'discovery': {
                'repeat':   int,            # how often to repeat a discovery action
                'timeout':  int,            # how long wait between
                'agents': {     # Configuration information for individual agent types,
                                # optionally including machine
                                str:{
                                    'repeat':   int, # repeat for this particular agent
                                    'timeout':  int  # timeout for this particular agent
                                },
                },
        },
        'monitoring': { 
                'repeat':   int,    # Default repeat interval in seconds
                'timeout':  int,
                'agents': {         # Configuration information for individual agent types,
                                    # optionally including machine
                                str:{
                                    'repeat':   int, # repeat for this particular agent
                                    'timeout':  int  # timeout for this particular agent
                                },
                },
        },
        'heartbeats':   {
            'repeat':   int,    # how frequently to heartbeat - in seconds
            'warn':     int,    # How long to wait when issuing a late heartbeat warning
            'dead':     int,    # How long to wait before declaring a system dead
        },
    }
    # This is the default configuration for the Assimilation project CMA
    # It should conform to the default_template above
    default_config = {
        'OUI': {            # Addendum of locally-known OUIs
                'b0-79-3c': 'Revolv, Inc.',
                '18-0c-ac': 'Canon, Inc.',
                'cc-3a-61': 'SAMSUNG ELECTRO MECHANICS CO., LTD.',
                'd8-50-e6': 'ASUSTek COMPUTER INC.',
        },
        'optional_modules':     [  # List of optional modules to be included
                                    'discoverylistener',
                                    'linkdiscovery',
                                    'checksumdiscovery',
                                    'monitoringdiscovery',
                                    'arpdiscovery',
                                ],
        'contrib_modules':          [],  # List of contrib modules to be included
        'cmaport':                  1984,                       # Our listening port
        'cmainit':                  pyNetAddr("0.0.0.0:1984"),  # Our listening address
        'compression_threshold':    20000,
        'compression_method':       "zlib",
        'discovery': {
                'repeat':           15*60,  # Default repeat interval in seconds
                'timeout':          300,    # Default timeout interval in seconds
                'agents': {         # Configuration information for individual agent types,
                                    # optionally including machine
                                    "checksumdiscovery": {'repeat':3600*8, 'timeout': 10*60},
                                    # "arpdiscovery/servidor":               {'repeat': 60},
                },
        },
        'monitoring': { 
                'repeat':           120,    # Default repeat interval in seconds
                'timeout':          180,    # Default repeat interval in seconds
                'agents': {         # Configuration information for individual agent types,
                                    # optionally including machine
                                    # "lsb::ssh":               {'repeat': int, 'timeout': int},
                                    # "ocf::Neo4j/servidor":    {'repeat': int, 'timeout': int},
                },
        },
        'heartbeats':   {
            'repeat':   1,    # how frequently to heartbeat - in seconds
            'warn':     5,    # How long to wait when issuing a late heartbeat warning
            'dead':     30,   # How long to wait before declaring a system dead
        },
    }
    def __init__(self, filename=None, template=None, defaults=None):
        'Init function for ConfigFile class, give us a filename!'
        if template is None:
            template = ConfigFile.default_template
        self.template = template
        if defaults is None:
            defaults = ConfigFile.default_config
        self.defaults = defaults
        self.config = pyConfigContext(filename=filename)

    def __contains__(self, name):
        "We're basically a dict lookalike - implement __contains__"
        return name in self.config

    def __getitem__(self, name):
        "We're basically a dict lookalike - implement __getitem__"
        return self.config[name]

    def __setitem__(self, name, value):
        "We're basically a dict lookalike - implement __setitem__"
        self.config[name] = value
        valid = self.isvalid()
        if not valid[0]:
            raise ValueError(valid[1])


    @staticmethod
    def _merge_config_elems(defaults, config):
        '''Merge data from our defaults into the configuration.
        Any element which is not included in the specified configuration is pulled
        from the default value for that element.
        NOTE that if you override a name which has an array for a value, you are
        eliminating all the array values.  The arrays are NOT somehow merged.
        '''
        for elem in defaults:
            delem = defaults[elem]
            if isinstance(delem, dict):
                if elem not in config:
                    config[elem] = pyConfigContext()
                ConfigFile._merge_config_elems(delem, config[elem])
            elif elem not in config:
                #print 'SETTING elem %s to delem %s' % (elem, delem)
                config[elem] = delem
    
    def complete_config(self):
        '''Create a complete configuration by merging with defaults
        and validating the merged config against our template.'''
        ConfigFile._merge_config_elems(self.defaults, self.config)
        ret = self.isvalid(self.config)
        if ret[0]:
            return self.config
        else:
            raise ValueError(ret[1])


    def isvalid(self, config=None):
        '''Validate the given configuration against our template.
        Return is a Tuple (True/False, 'explanation of errors')'''
        if config is None:
            config = self.default_config
        return ConfigFile._check_validity(self.template, config)

    @staticmethod
    def _check_validity(template, configobj):
        '''Recursively validate a complex dict-like object against a complex template object
        This is an interesting, but somewhat complex operation.
        There are many ways to fail, hence many failure-case return statements, and
        even 4 success-case returns...
        This _could_ be split up.  Not obvious it's a win to do so...

        Return value is a Tuple (True/False, 'explanation of errors')
        '''

        if isinstance(template, (type, ClassType)):
            return ConfigFile._check_validity_type(template, configobj)
        if isinstance(template, dict):
            return ConfigFile._check_validity_dict(template, configobj)
        if isinstance(template, (list, tuple)):
            return ConfigFile._check_validity_list(template, configobj)
        if isinstance(template, set):
            return ConfigFile._check_validity_set(template, configobj)

        return (False, "Case we didn't allow for: %s vs %s" % (str(template), str(configobj)))

    @staticmethod
    def _check_validity_type(template, configobj):
        'Make sure the configobj is of the given type'
        if (not isinstance(configobj, template)):
            return (False, '%s is not of %s' % (configobj, template))
        return (True, '')

    @staticmethod
    def _check_validity_set(template, configobj):
        'Make sure the configobj is of a string matching something in the set'
        if configobj not in template:
            return (False, '%s is not in %s' % (configobj, template))
        return (True, '')

    @staticmethod
    def _check_validity_dict(template, configobj):
        'Check a configobj for validity against a "dict" template'
        try:
            keys = configobj.keys()
        except AttributeError:
            return (False, '%s is not a dict' % (configobj))
        #   Were we just given "str" as a key value?
        #   If so, then any names are legal, but the values all have to be the "correct" type
        if str in template:
            validatetype = template[str]
            # Any key is fine, but elements have to match the given type
            for configkey in keys:
                ret = ConfigFile._check_validity(validatetype, configobj[configkey])
                if not ret[0]:
                    return (False, 'Element %s: %s' % (configkey, ret[1]))
        else:
            # Every key in the configobj must also be in the template
            for elemname in keys:
                if elemname not in template:
                    return (False, ('%s is not a legal element' % (elemname)))
                ret = ConfigFile._check_validity(template[elemname], configobj[elemname])
                if not ret[0]:
                    return (False, 'Element %s: %s' % (elemname, ret[1]))
        return (True, '')


    @staticmethod
    def _check_validity_list(template, configobj):
        'Check a configobj for validity against a list/tuple template'
        # When the template element is a list or tuple, then the item has to be a
        # list or tuple and every element of the item has to match the given template
        # Note that all lists (currently) have to be of the same type...
        checktype = template[0]
        if not isinstance(configobj, (list, tuple)):
            return (False, ('%s is not a list or tuple' % (configobj)))
        for elem in configobj:
            ret = ConfigFile._check_validity(checktype, elem)
            if not ret[0]:
                return (False, ('Array element: %s' % ret[1]))
        return (True, '')

if __name__ == '__main__':
    cf = ConfigFile()
    isvalid = cf.isvalid(ConfigFile.default_config)
    print 'DEFAULT CONFIG valid?:', isvalid
    print 'Complete config:', cf.complete_config()  # checks for validity
