{
  if ($0 ~ /^---/) {} # skip ------
  else if ($0 ~ /^$/) {} # skip empty lines
  else if ($1 ~ /^r[0-9]+/) { # version label
    print "# 0.3.1." substr($1, 2) " (" $5 ")" 
  } else { # commit message
    gsub("#[0-9]+ *", "");
    print "- " $0
  }
}
