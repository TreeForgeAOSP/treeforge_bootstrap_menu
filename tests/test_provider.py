from __future__ import annotations
import json
from pathlib import Path
import re
import subprocess
import unittest
ROOT=Path(__file__).resolve().parents[1]
class ProviderTests(unittest.TestCase):
    def test_verify(self):
        subprocess.run([str(ROOT/'menuctl'),'verify'],check=True,cwd=ROOT)
    def test_readme_single_version(self):
        values={}
        for raw in (ROOT/'VERSION').read_text().splitlines():
            if '=' in raw:
                k,v=raw.split('=',1); values[k]=v
        version=values['TREEFORGE_BOOT_MENU_VERSION']
        text=(ROOT/'README.md').read_text()
        self.assertEqual(text.splitlines()[0],f'# TreeForge Bootstrap Menu — {version}')
        self.assertEqual(re.findall(r'v1\.0b[0-9]+',text),[version])
    def test_component_identity(self):
        data=json.loads((ROOT/'provider.json').read_text())
        self.assertEqual(data['provider'],'treeforge_bootstrap_menu')
        self.assertEqual(data['abi'],1)
    def test_profile(self):
        data=json.loads((ROOT/'profiles/treeforge-default.json').read_text())
        self.assertEqual(data['profile_id'],'treeforge-default')
        self.assertEqual(data['schema'],1)
if __name__=='__main__': unittest.main()
