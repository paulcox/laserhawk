#!/usr/bin/perl

use Linux::Input::Joystick;
use YAML;

  my $js = Linux::Input::Joystick->new('/dev/input/js1');
  while (1) {
    my @event = $js->poll(0.01);
    print Dump($_) foreach (@event);
  }
