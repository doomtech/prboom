#!/usr/bin/perl -wT
#
# $Id: genlin.cgi,v 1.1 2000/11/26 14:18:29 cph Exp $
#
# Copyright (C) 2000 by Colin Phipps <cphipps@doomworld.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#1. Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#2. Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#3. Neither the name of the author nor the names of its contributors
#   may be used to endorse or promote products derived from this software
#   without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

use strict;
use CGI qw(:standard);

my %field_name = (
	delay => 'Wait delay (seconds)',
	kind => 'Mechanism',
	locked_kind => 'Mechanism',
	lock => 'Needs keys',
	monster => 'Monsters can open',
	skck => 'Skull keys and card keys considered equivalent',
	speed => 'Speed',
	repeatable => 'Repeatable',
	trigger => 'Trigger',
	);

my $bool = [ qw/No Yes/ ];

my %field_value = (
	delay => [ 1, 4, 9, 30],
	kind => [ 'Open-wait-close', 'Open and stay',
		'Close-wait-open', 'Close and stay' ],
	locked_kind => [ 'Open-wait-close', 'Open and stay'],
	lock => [ 'Any Key',
		'Red Key', 'Blue Key', 'Yellow Key',
		'Red Skull', 'Blue Skull', 'Yellow Skull',
		'All keys' ],
	monster => $bool,
	skck => $bool,
	speed => [ qw/Slow Normal Fast Turbo/ ],
	repeatable => $bool,
	trigger => [ qw/ Walk Switch Shoot Door / ],
	);

my %line_classes = (
	Door => {
		start => 0x3c00,
		fields => {
			repeatable => 0x1,
			trigger => 0x6,
			speed => 0x18,
			kind => 0x60,
			monster => 0x80,
			delay => 0x300,
			},
	},
	);

sub mshift ($) {
	my $mask = shift;
	my $j;
	for ($j = 0; !(($mask >> $j) & 1); $j++) { }
	return $j;
}

sub bad ($) {
	print p($_[0]), end_html;
	exit 0;
}

# Allow source download
if (param('source')) {
	open(SELF,$0) or die "Can't open self";
	print header(-type=>'application/x-perl',-expires=>'+7d');
	while (defined (my $l = <SELF>)) { print $l; };
	close SELF;
	exit 0;
}
print header, start_html('Boom generalised linedef calculator');


print p('This program written by <a href="mailto:cphipps@doomworld.com">Colin Phipps</a>. You can ',
	a({href=> (url(-relative=>1) . '?source=1')},"download the source"),
	'.'),hr;

my $class = param('class');
my $c = $class ? $line_classes{$class} : undef;

if ($c) {
	# User has specified the class, so we're at the second page
	# now, selecting detailed properties. Run through the 
	# properties creating the form fields.
	print start_form(-method => 'GET'),
		p("Select properties of this \L$class\E"),
		hidden('class',$class);
	foreach my $f (keys %{$c->{fields}}) {
		print "<p>", $field_name{$f}, ": ";
		if ($field_value{$f} eq $bool) {
			print checkbox(-name=>$f,-label=>"",-value=>1);
		} else {
			my %label_map;
			my $num = @{$field_value{$f}} - 1;
			@label_map{0..$num} = @{$field_value{$f}};
			print popup_menu(-name=>$f,
				-values=>[0..$num],
				-labels=>\%label_map);
		}
		print "</p>";
	}
	print submit('Done'),end_form;
	if (param('Done')) {
		my $linenum = $c->{start};
		foreach my $f (keys %{$c->{fields}}) {
			my $val = param($f) || 0;
			$linenum += $val << mshift($c->{fields}{$f});
		}
		print p("Linedef number: ", em($linenum), sprintf("(0x%x)",$linenum)) if defined $linenum;
	}
	print hr;
}
# Lack of params indicates we must be at the welcome screen
print start_form(-method => 'GET'),
	p("Class of line: ",
	popup_menu(-name=>'class', -values => [keys %line_classes])),
	submit('Make new line'), end_form;

print p(a({href => url(-relative=>1)}, "Start again")),
	hr,p("Copyright (C) 2000 Colin Phipps"),end_html;

