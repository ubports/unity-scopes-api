from os import path
import subprocess, json, configparser

# map 'name' from click package manifest to LP project (if known)
click_name_to_src = {
    'com.ubuntu.scopes.youtube': 'unity-scope-youtube',
    'com.ubuntu.scopes.vimeo': 'unity-scope-vimeo'
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
        dpkg_find_file = subprocess.check_output(['dpkg', '-S', inifile], universal_newlines=True)
        pkg = dpkg_find_file.split(':')[0]
        srcpkg = None
        ver = ''
        # parse 'apt-cache show' output to find actual package and source package of affected scope
        pkg_info = subprocess.check_output(['apt-cache', 'show', pkg], universal_newlines=True)
        for line in pkg_info.splitlines():
            if not line.startswith(' '):
                vals = line.split(sep=':')
                if len(vals) == 2:
                    if vals[0] == 'Source':
                        srcpkg = vals[1].strip()
                    if vals[0] == 'Version':
                        ver = vals[1].strip()
        if not srcpkg:
            srcpkg = pkg
        report['Package'] = '%s %s' % (pkg, ver)
        report['SourcePackage'] = srcpkg
    except:
       pass

def collect_click_info(report, inifile):
    try:
        # get click manifest of a click package that owns this inifile
        click_info = subprocess.check_output(['click', 'info', inifile], universal_newlines=True)
        manifest = json.loads(click_info)
        report['Package'] = '%s %s' % (manifest['name'], manifest['version'])
        name = manifest['name']
        if name in click_name_to_src:
            report['SourcePackage'] = click_name_to_src[name]
        else:
            report['SourcePackage'] = name
        report['PackageArchitecture'] = manifest['architecture']
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
