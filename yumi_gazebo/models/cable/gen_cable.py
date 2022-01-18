#!/usr/bin/python3
# -*- coding:utf-8 -*-

#from xml.etree import ElementTree as  etree
from xml.etree.ElementTree import Element
from xml.etree.ElementTree import SubElement
from xml.etree.ElementTree import ElementTree

from xml_utility import prettify, SubElementText, l2s

class cable_creator():
    def __init__(self, section_vol):
        self.vol = section_vol
        # generate root node
        self.sdf = Element('sdf', {'version':'1.5'})

        # generate first child-node head
        self.model = SubElement(self.sdf, 'model', {'name':'cable'})


    def add_section(self, no, pose):
        style = 'box'

        link = SubElement(self.model, 'link', {'name':'link_' + str(no)})
        SubElementText(link, 'gravity', _text = 'true')
        SubElementText(link, 'pose', _text = l2s(pose))
        
        # add collision section
        collision = SubElement(link, 'collision', {'name':'link_' + str(no) + '_collision'})
        geo_collision = SubElement(collision, 'geometry')
        # volume is either a cylinder, a box, a triangular, etc...
        vol_collision = SubElement(geo_collision, style)
        SubElementText(vol_collision, 'size', _text = l2s(self.vol))

        # add visual section
        visual = SubElement(link, 'visual', {'name':'link_' + str(no) + '_visual'})
        geo_visual = SubElement(visual, 'geometry')
        vol_visual = SubElement(geo_visual, style)
        SubElementText(vol_visual, 'size', _text = l2s(self.vol))

    def add_joint(self, no):
        # define two sphere with joints attached to each of them.
        joint = SubElement(self.model, 'joint', {'type':'ball', 'name':'joint_' + str(no-1) + '_' + str(no)})
        offset = [-self.vol[0]/2, 0, 0, 0, 0, 0]
        SubElementText(joint, 'pose', _text = l2s(offset))
        SubElementText(joint, 'child', _text = 'link_' + str(no))
        SubElementText(joint, 'parent', _text = 'link_' + str(no-1))

    def create_xml(self, n):
        # pose = SubElement(model, 'pose')
        # pose.text = '0 0 10 0.708 0 0'
        SubElementText(self.model, 'static', _text='false')
        SubElementText(self.model, 'self_collide', _text='true')

        # a section is cylinder*1+sphere*2 (use 2 joints to create a universal one)
        self.add_section(0, [0]*6)
        joint = SubElementText(self.model, 'joint', {'name':'joint_0_world', 'type':'fixed'})
        SubElementText(joint, 'pose', _text=l2s([0]*6))
        SubElementText(joint, 'child', _text='link_0')
        SubElementText(joint, 'parent', _text='world')
        for i in range(1,n):
            self.add_section(i, [self.vol[0]*i, 0, 0, 0, 0, 0])
            self.add_joint(i)

        prettify(self.sdf, '  ', '\n')

        tree = ElementTree(self.sdf)

        # write out xml data
        tree.write('model.sdf', encoding = 'utf-8', xml_declaration = True)

if __name__ == '__main__':
    creator = cable_creator([0.02, 0.005, 0.005])
    creator.create_xml(50)