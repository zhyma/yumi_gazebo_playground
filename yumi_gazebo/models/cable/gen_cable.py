#!/usr/bin/python3
# -*- coding:utf-8 -*-

#from xml.etree import ElementTree as  etree
from xml.etree.ElementTree import Element
from xml.etree.ElementTree import SubElement
from xml.etree.ElementTree import ElementTree

from xml_utility import prettify, SubElementText, l2s
from math import pi as pi

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

        # add inertial section
        inertial = SubElement(link, 'inertial')
        if no == 0:
            # The first section is a counterweight, 
            # which is much heavier than other parts
            # rho = 50*8.96*1000
            rho = 8.96*1000
        else:
            # suppose we are using copper, 8.96g/cm^3
            rho = 8.96*1000 
        mass = self.vol[0]*pi*(self.vol[1]/2)**2*rho
        SubElementText(inertial, 'mass', _text = str(mass))
        # "inertia" is inside of "inertial"
        inertia = SubElement(inertial, 'inertia')
        # Solid cylinder of radius r, height h and mass m
        # ixx=(ml^2)/3, ixy=0, ixz=0, iyy=0, iyz=0, izz=(ml^2)/3
        # taking a long box as a long rod (along x axis)
        # the first element is the length of the boxy stick or the rod
        h = self.vol[0]
        r = self.vol[1]/2
        ir = mass*(3*r*r+h*h)/12
        ih = mass*r*r/2
        SubElementText(inertia, 'ixx', _text = str(ih))
        SubElementText(inertia, 'ixy', _text = '0')
        SubElementText(inertia, 'ixz', _text = '0')
        SubElementText(inertia, 'iyy', _text = str(ir))
        SubElementText(inertia, 'iyz', _text = '0')
        SubElementText(inertia, 'izz', _text = str(ir))

        # print(mass, end=', ')
        # print(self.vol[0], end = ',')
        # print(i_val)
        
        # add collision section
        collision = SubElement(link, 'collision', {'name':'link_' + str(no) + '_collision'})
        geo_collision = SubElement(collision, 'geometry')
        # volume is either a cylinder, a box, a triangular, etc...
        vol_collision = SubElement(geo_collision, style)
        SubElementText(vol_collision, 'size', _text = l2s(self.vol))
        # surface = SubElement(collision, 'surface')
        # friction = SubElement(surface, 'friction')
        # ode = SubElement(friction, 'ode')
        # mu = 0.5
        # SubElementText(ode, 'mu', _text=str(mu))
        # SubElementText(ode, 'mu2', _text=str(mu))


        # add visual section
        visual = SubElement(link, 'visual', {'name':'link_' + str(no) + '_visual'})
        geo_visual = SubElement(visual, 'geometry')
        vol_visual = SubElement(geo_visual, style)
        SubElementText(vol_visual, 'size', _text = l2s(self.vol))
        material = SubElement(visual, 'material')
        script = SubElement(material, 'script')
        SubElementText(script, 'uri', _text = 'file://media/materials/scripts/gazebo.material')
        SubElementText(script, 'name', _text = 'Gazebo/Green')

    def add_joint(self, no):
        # define two sphere with joints attached to each of them.
        joint = SubElement(self.model, 'joint', {'type':'universal', 'name':'joint_' + str(no-1) + '_' + str(no)})
        offset = [-self.vol[0]/2, 0, 0, 0, 0, 0]
        SubElementText(joint, 'pose', _text = l2s(offset))
        SubElementText(joint, 'child', _text = 'link_' + str(no))
        SubElementText(joint, 'parent', _text = 'link_' + str(no-1))
        # damping = 0.5
        # friction = 0.5
        # axis = SubElement(joint, 'axis')
        # SubElementText(axis, 'xyz', _text='0 0 1')
        # dynamics = SubElement(axis, 'dynamics')
        # SubElementText(dynamics, 'damping', _text = str(damping))
        # SubElementText(dynamics, 'friction', _text = str(friction))
        # limit = SubElement(axis, 'limit')
        # SubElementText(limit, 'lower', _text = str(-pi/2))
        # SubElementText(limit, 'upper', _text = str(pi/2))
        # axis2 = SubElement(joint, 'axis2')
        # dynamics2 = SubElementText(axis2, 'dynamics')
        # SubElementText(dynamics2, 'damping', _text = str(damping))
        # SubElementText(dynamics2, 'friction', _text = str(friction))
        # # limit2 = SubElement(axis2, 'limit')
        # # SubElementText(limit2, 'lower', _text = str(-pi/2))
        # # SubElementText(limit2, 'upper', _text = str(pi/2))

    def create_xml(self, n):
        # pose = SubElement(model, 'pose')
        # pose.text = '0 0 10 0.708 0 0'
        SubElementText(self.model, 'static', _text='false')
        SubElementText(self.model, 'self_collide', _text='true')

        # a section is cylinder*1+sphere*2 (use 2 joints to create a universal one)
        self.add_section(0, [0]*6)
        # joint = SubElementText(self.model, 'joint', {'name':'joint_0_world', 'type':'fixed'})
        # SubElementText(joint, 'pose', _text=l2s([0]*6))
        # SubElementText(joint, 'child', _text='link_0')
        # SubElementText(joint, 'parent', _text='world')
        for i in range(1,n):
            self.add_section(i, [self.vol[0]*i, 0, 0, 0, 0, 0])
            self.add_joint(i)

        prettify(self.sdf, '  ', '\n')

        tree = ElementTree(self.sdf)

        # write out xml data
        tree.write('model.sdf', encoding = 'utf-8', xml_declaration = True)

if __name__ == '__main__':
    # [x, y, z], y=z\approx estimated radians
    # use boxes to replace cylinders
    creator = cable_creator([0.01, 0.005, 0.005])
    creator.create_xml(70)