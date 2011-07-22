require 'rake/clean'

begin
  require 'psych'
rescue LoadError
ensure
  require 'yaml'
end

root = File.expand_path File.dirname(__FILE__)

config = YAML.load_file File.join(root, 'config.yaml')

unless File.exists?('config.yaml') && File.exists?('config.h')
  STDERR.puts "Please run 'ruby configure' first"
  abort
end

# load rake helper tasks
Dir.glob("#{root}/rakelib/**/*.rake").sort.each do |lib|
  puts "Loading #{File.basename(lib)}" if Rake.application.options.trace
  load lib
end

# FIXME why aren't all these files being deleted?
CLOBBER.include('config.h', 'config.yaml', 'xval.exe')

# TODO refactor with configure until hardcode clean
file 'xval.exe' => ['src/xvalid.c'] do |t|
  inc_dirs = config[:include_dirs].map { |i| "-I#{i}" }.join(' ')
  lib_dirs = config[:library_dirs].map { |i| "-L#{i}" }.join(' ')
  defines = config[:defines].map { |i| "-D#{i}" }.join(' ')

  cmd = %[cmd.exe /c "#{config[:cc]} ]
  cmd << %[#{defines} ]
  cmd << %[-Wall #{config[:optflags]} #{config[:debugflags]} ]
  cmd << %[-I. #{inc_dirs} #{lib_dirs} ]
  cmd << %[-s -o #{t.name} #{t.prerequisites.join} ]
  cmd << %[-Wl,-static -lxml2 -Wl,-dy -lws2_32"]
  sh cmd
end

desc 'Build xval.exe'
task :build => ['xval.exe']

task :default => [:build]
