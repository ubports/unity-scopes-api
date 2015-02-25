#
# Copyright (C) 2015 Canonical Ltd.
# Author: Pawel Stolowski <pawel.stolowski@canonical.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


#
# This script parses ProcCmdline to check if this is a crash of scoperunner process.
# If so, it checks what scope made it crash and adjusts Package and SourcePackage
# attributes of the crash report to point at the crashing scope.
#

from os import path
import subprocess, json, configparser
import apport
import apport.fileutils
from apport.packaging_impl import impl as packaging

# map 'name' from click package manifest to LP project (if known)
click_name_to_lp = {
    'com.ubuntu.scopes.youtube': 'unity-scope-youtube',
    'com.ubuntu.scopes.vimeo': 'unity-scope-vimeo',
    'com.ubuntu.scopes.soundcloud': 'unity-scope-soundcloud'
}

def get_scope_display_name(ini_file_path):
    try:
        cfg = configparser.ConfigParser()
        cfg.read(ini_file_path)
        return cfg.get('ScopeConfig', 'DisplayName')
    except:
        pass
    return ''

def collect_dpkg_info(report, inifile):
    try:
        # find deb package that owns this inifile
        pkg = apport.fileutils.find_file_package(inifile)
        ver = packaging.get_version(pkg)
        report['Package'] = '%s %s' % (pkg, ver)
        report['SourcePackage'] = packaging.get_source(pkg)
    except:
       pass

def collect_click_info(report, inifile):
    try:
        # get click manifest of a click package that owns this inifile
        click_info = subprocess.check_output(['click', 'info', inifile], universal_newlines=True)
        manifest = json.loads(click_info)
        name = manifest['name']
        report['Package'] = '%s %s' % (name, manifest['version'])
        report['SourcePackage'] = name
        report['PackageArchitecture'] = manifest['architecture']
        lp_project = None
        if name in click_name_to_lp:
            lp_project = click_name_to_lp[name]
        elif 'Canonical Content Partners' in manifest['maintainer'] and 'canonical' in name:
            lp_project = 'savilerow'
        if lp_project:
            report['CrashDB'] = '{"impl": "launchpad", "project": "' + lp_project + '", "bug_pattern_url": "http://people.canonical.com/~ubuntu-archive/bugpatterns/bugpatterns.xml"}'
    except:
        pass

def add_info(report, ui):
    exec_path = report.get('ExecutablePath')
    if not exec_path:
        return
    cmd_line = report.get('ProcCmdline')
    if not cmd_line:
        return
    if exec_path.find("scoperunner") < 0:
        return
    args = cmd_line.split()
    if len(args) != 2:
        return

    scope_ini_path = args[1]
    scope_name = get_scope_display_name(scope_ini_path)
    if scope_name:
        report['Scope'] = scope_name

    #
    # check whether it's a click package or deb and collect additional pkg info
    if scope_ini_path.find('.local/share') >= 0:
        report['ClickPackage'] = 'True'
        collect_click_info(report, scope_ini_path)
    else:
        collect_dpkg_info(report, scope_ini_path)
