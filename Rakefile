unless File.exists?('config.yaml') && File.exists?('config.h')
  STDERR.puts "Please run 'ruby configure' first"
  abort
end

require 'rake/clean'

root = File.expand_path File.dirname(__FILE__)

begin
  require 'psych'
rescue LoadError
ensure
  require 'yaml'
end

# TODO abort if malformed YAML
config = YAML.load_file File.join(root, 'config.yaml')

# load rake helper tasks
Dir.glob("#{root}/rakelib/**/*.rake").sort.each do |lib|
  puts "Loading #{File.basename(lib)}" if Rake.application.options.trace
  load lib
end

# FIXME why isn't config.yaml being deleted?
CLOBBER.include('config.h', 'config.yaml', config[:APPNAME])

# TODO refactor with configure until hardcode clean
file config[:APPNAME] => ['src/xvalid.c'] do |t|
  inc_dirs = config[:include_dirs].map { |i| "-I#{i}" }.join(' ')
  lib_dirs = config[:library_dirs].map { |i| "-L#{i}" }.join(' ')
  defines = config[:defines].map { |i| "-D#{i}" }.join(' ') if config[:defines]
  optflags = config[:optflags].map { |i| "-#{i}" }.join(' ') if config[:optflags]
  debugflags = config[:debugflags].map { |i| "-#{i}" }.join(' ') if config[:debugflags]

  cmd = %[#{config[:shell]} "#{config[:cc]} ]
  cmd << %[#{defines} ]
  cmd << %[-Wall #{optflags} #{debugflags} ]
  cmd << %[-I. #{inc_dirs} #{lib_dirs} ]
  cmd << %[#{config[:strip_all]} -o #{t.name} #{t.prerequisites.join} ]
  cmd << %[#{config[:libs]}"]
  sh cmd
end

desc 'Build xval.exe'
task :build => [ config[:APPNAME] ]

task :default => [:build]
