# Michel Grimminck 2016, copy freely

#  sudo python MDGImageLibSetup.py build
#  sudo python MDGImageLibSetup.py install

from distutils.core import setup, Extension

module1 = Extension('MDGImageLib',
                    sources = ['MDGImageLib.c'])

setup (name = 'MDGImageLib',
       version = '1.0',
       description = 'MDG Image Lib',
       ext_modules = [module1])
