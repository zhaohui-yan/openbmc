project('aspeed-pfr-tool', 'c', 'cpp',
        version : '1.0',
        )

openssl = dependency('openssl', required : true)
# i2c-tools doesn't ship a pkg-config file for libi2c
i2c = meson.get_compiler('cpp').find_library('i2c')
aspeed_pfr_tool_dependencies = [openssl, i2c]

# Generate the aspeed-pfr-tool executable
executable(
    'aspeed-pfr-tool',
    [
      'i2c_utils.c',
      'provision.c',
      'main.c'
    ],
    dependencies: aspeed_pfr_tool_dependencies,
    install: true
)

