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

begin
  config = YAML.load_file File.join(root, 'config.yaml')
rescue ::Exception
  abort 'Unable to parse config.yaml, exiting...'
end

# load rake helper tasks
Dir.glob("#{root}/rakelib/**/*.rake").sort.each do |lib|
  puts "Loading #{File.basename(lib)}" if Rake.application.options.trace
  load lib
end

def ext(fn, newext)
  fn.sub(/\.[^.]+$/, newext)
end

SRCFILES = Rake::FileList['src/*.c']
OBJFILES = SRCFILES.map { |f| File.basename ext(f, '.o') }

# FIXME why isn't config.yaml being deleted?
CLEAN.concat OBJFILES
CLOBBER.include('config.h', 'config.yaml', config[:APPNAME])

OBJFILES.each do |obj|
  src = File.join('src', ext(obj, '.c'))
  file obj => src do |t|
    cmd = %[#{config[:shell]} "#{config[:cc]} ]
    cmd << %[#{$opts[:defines]} ]
    cmd << %[-Wall #{$opts[:optflags]} #{$opts[:debugflags]} ]
    cmd << %[-I. #{$opts[:inc_dirs]} -c -o #{t.name} #{t.prerequisites.join}"]
    sh cmd
  end
end

task :settings => ['config.yaml'] do |t|
  $opts = {
    :inc_dirs => config[:include_dirs].map { |i| "-I#{i}" }.join(' '),
    :lib_dirs => config[:library_dirs].map { |i| "-L#{i}" }.join(' '),
  }
  $opts[:defines] = config[:defines].map { |i| "-D#{i}" }.join(' ') if config[:defines]
  $opts[:optflags] = config[:optflags].map { |i| "-#{i}" }.join(' ') if config[:optflags]
  $opts[:debugflags] = config[:debugflags].map { |i| "-#{i}" }.join(' ') if config[:debugflags]
end

file config[:APPNAME] => OBJFILES do |t|
  cmd = %[#{config[:shell]} "#{config[:cc]} ]
  cmd << %[-Wall #{$opts[:optflags]} #{$opts[:debugflags]} ]
  cmd << %[#{$opts[:lib_dirs]} ]
  cmd << %[#{config[:strip_all]} -o #{t.name} #{t.prerequisites.join(' ')} ]
  cmd << %[#{config[:libs]}"]
  sh cmd
end

desc "Build #{config[:APPNAME]} v#{config[:VERSION]}"
task :build => [:settings, config[:APPNAME]]

task :default => [:build]
