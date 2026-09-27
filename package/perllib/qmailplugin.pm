package qmailplugin;

use strict;
use warnings;

use Carp qw(carp croak);
use charnames();

use Exporter 'import';
our @EXPORT = qw(
	qp_do		qp_defer	qp_softerror
	qp_LM		qp_unset	qp_safestring
	qp_LR		qp_error	qp_changeaddress
	qp_LD		qp_header	qp_skip_plugins_for_command
	qp_set		qp_reject	qp_skip_plugins_for_message
	qp_drop		qp_message	qp_unconditional_accept_command
	qp_init
);

our ($stream, $debuglevel, $debug);

sub def_debug { carp '"', join('", "', map { $_ // '<undef>' } @_ ), '"' ; 1};
sub qp_init {			#export
    my ($s, $dl, $df) = @_;
    $debuglevel = $dl // 0;
    $debug = $df // \&def_debug;
    $stream = $s ;
}
qp_init(); # hopefully overwritten by caller

our %when;
{
    sub add_when {
	my $act = shift;
	$when{$_} = $act for split ", ", lc shift;
    }
    add_when('M', 'm, mail, mail from, mail from:, f, from, from:');
    add_when('R', 'r, rcpt, rcpt to, rcpt to:, t, to, to:');
    add_when('D', 'd, data, data:');
}
sub shuffle_args {
    my $code;
    my $v = shift;
    if ($v =~ m/^[2345][0-5][0-9]$/) {
	$code = $v;
	$v = shift;
    }
    return $v =~ m/^[245]\.[0-7]\.[0-9]+$/ 
	? ($code, $v, join(' ', grep { defined $_ } @_))
	: ($code, undef, join(' ', grep {defined $_} ($v, @_) ));
}	
sub qp_safestring {		# export
    my $str = join '', map { $_ // '<undef>' } @_ ;
    return $str =~ s/\n$//r
        =~ s/([\\\"\'^])/\\$1/gr
        =~ s/([\p{Cc}])/ lc charnames::unctrl($1) /gre;
}
sub qp_do {			# export
    my ($action, $content) = @_;
    print $stream, $action . qp_safestring($content) . "\n";
}
sub dosplit {
    my ($action,$code,$subcode,$message) = @_;
    $code=550 unless defined $code;
    $message = $subcode . ' ' . $message if defined $subcode;
    $message =~ s/\n{2,}/\n/g;
    $message =~ s/\n$//;
    return gp_do($action, $code . ' ')
	unless $message;
    my @lines = split ("\n", $message);
    gp_do('P', $code. '-' . $_) for (@lines[0..@lines-2]);
    return gp_do($action, $code . ' ' . @lines[@lines - 1]);
}
sub qp_skip_plugins_for_message { # export
    qp_do('A');
    exit 0;
}
sub qp_skip_plugins_for_command { # export
    qp_do('N');
    exit 0;
}
sub qp_unconditional_accept_command { # export
    qp_do('O');
    exit 0;
}
sub qp_error {			# export
    my ($code, $subcode, $message) = shuffe_args(@_);
    qp_dosplit('E', $code // '550', $subcode, $message);
    exit 1;
}
sub qp_softerror {		# export
    my ($code, $subcode, $message) = shuffe_args(@_);
    qp_dosplit('E', $code // '451', $subcode, $message);
    exit 1;
}
sub qp_defer {			# export
    my $when = shift;
    my ($code, $subcode, $message) = shuffe_args(@_);
    qp_error(451, '5.3.5', 'unrecognised protocol stage "' . $when . '"')
	unless $when = $when{$when};
    qp_do("L$when", $code, $subcode, $message);
}
sub qp_LM {			# export
    qp_defer('m', @_);
}
sub qp_LR {			# export
    qp_defer('r', @_);
}
sub qp_LD {			# export
    qp_defer('d', @_);
}
sub qp_reject {			# export
    my ($code, $subcode, $message) = shuffe_args(@_);
    qp_dosplit('R', $code // '550', $subcode, $message);
    exit 1;
}
sub qp_drop {			# export
    qp_do('D');
    exit 1;
}
sub qp_set {			# export
    my $var = shift or croak "variable must have a name";
    $var = qp_safestring($var);
    my $val = shift;
    if (defined $val) {
	$var =~ s/=/\\075/g;
	$var .= '=' . $val;
    }
    qp_do('S', $var);
}
sub qp_unset {			# export
    my $var = shift or croak "variable must have a name";
    $var =~ s/=/\\075/g;
    qp_do('U', $var);
}
sub qp_header {			# export
    my $header = shift;
    qp_do('H', $header);
}
sub qp_changeaddress {		# export
    my $address = shift;
    qp_do('C', $address);
}
sub qp_message {		# export
    my $message = shift;
    gp_dosplit('P', $message);
}


# awk '/^sub qp_.*{.*#.*export/ { printf "%s ",$2 }' qmailplugin.pm

1;
