import { useState } from 'preact/hooks';

const functionOptions = [
  { value: 0, label: 'None' },
  { value: 1, label: 'Main Power' },
  { value: 2, label: 'Power LED' },
  { value: 3, label: 'Grinder' },
];

export function ShellyCard({
  enabled,
  grinderEnabled,
  ledEnabled,
  mainPowerEnabled,
  ledMode,
  onToggle,
  onLedModeChange,
  devices,
  assignments,
  scanResults,
  scanError,
  onScan,
  onAddDevice,
  onRemoveDevice,
  onTestDevice,
  onAssignmentChange,
}) {
  const [authInputs, setAuthInputs] = useState({});

  const handleAuthInput = (host, field, value) => {
    setAuthInputs(prev => ({
      ...prev,
      [host]: {
        ...prev[host],
        [field]: value,
      },
    }));
  };

  const getAssignmentValue = (deviceId, channel) => {
    const match = assignments.find(a => a.deviceId === deviceId && a.channel === channel);
    return match ? match.function : 0;
  };

  const handleAssign = (deviceId, channel, value) => {
    onAssignmentChange(deviceId, channel, parseInt(value, 10));
  };

  const filteredOptions = option => {
    if (option.value === 1 && !mainPowerEnabled) {
      return false;
    }
    if (option.value === 2 && !ledEnabled) {
      return false;
    }
    if (option.value === 3 && !grinderEnabled) {
      return false;
    }
    return true;
  };

  return (
    <div className='bg-base-200 rounded-lg p-4 space-y-4'>
      <div className='flex items-center gap-3'>
        <input
          id='shellyEnabled'
          name='shellyEnabled'
          value='shellyEnabled'
          type='checkbox'
          className='toggle toggle-primary'
          checked={!!enabled}
          onChange={onToggle('shellyEnabled')}
          aria-label='Enable Shelly plugin'
        />
        <span className='text-xl font-medium'>Shelly Plugin (Gen2+)</span>
      </div>

      {enabled && (
        <div className='border-base-300 space-y-4 border-t pt-4'>
          <div className='grid gap-3 md:grid-cols-3'>
            <label className='flex items-center gap-2 text-sm'>
              <input
                id='shellyGrinderEnabled'
                name='shellyGrinderEnabled'
                value='shellyGrinderEnabled'
                type='checkbox'
                className='toggle toggle-primary'
                checked={!!grinderEnabled}
                onChange={onToggle('shellyGrinderEnabled')}
              />
              <span>Grinder Support</span>
            </label>
            <label className='flex items-center gap-2 text-sm'>
              <input
                id='shellyLedEnabled'
                name='shellyLedEnabled'
                value='shellyLedEnabled'
                type='checkbox'
                className='toggle toggle-primary'
                checked={!!ledEnabled}
                onChange={onToggle('shellyLedEnabled')}
              />
              <span>Power LED</span>
            </label>
            <label className='flex items-center gap-2 text-sm'>
              <input
                id='shellyMainPowerEnabled'
                name='shellyMainPowerEnabled'
                value='shellyMainPowerEnabled'
                type='checkbox'
                className='toggle toggle-primary'
                checked={!!mainPowerEnabled}
                onChange={onToggle('shellyMainPowerEnabled')}
              />
              <span>Main Power Schedule</span>
            </label>
          </div>

          {ledEnabled && (
            <div className='form-control'>
              <label htmlFor='shellyLedMode' className='mb-2 block text-sm font-medium'>
                LED behavior
              </label>
              <select
                id='shellyLedMode'
                name='shellyLedMode'
                className='select select-bordered w-full'
                value={String(ledMode)}
                onChange={onLedModeChange}
              >
                <option value='0'>LED on with main power</option>
                <option value='1'>LED on only when active</option>
              </select>
            </div>
          )}

          <div className='space-y-3'>
            <div className='flex items-center justify-between'>
              <span className='text-lg font-medium'>Shelly Devices</span>
              <button type='button' onClick={onScan} className='btn btn-primary btn-sm'>
                Add Shelly (Scan LAN)
              </button>
            </div>
            {scanError && <p className='text-sm text-error'>{scanError}</p>}
            {scanResults.length > 0 && (
              <div className='space-y-2'>
                {scanResults.map(result => (
                  <div key={result.host} className='rounded border p-2 text-sm'>
                    <div className='flex flex-wrap items-center justify-between gap-2'>
                      <div>
                        <div className='font-medium'>
                          {result.name || 'Unknown'} ({result.model || 'Unknown'})
                        </div>
                        <div className='opacity-70'>{result.host}</div>
                      </div>
                      <button
                        type='button'
                        className='btn btn-secondary btn-xs'
                        onClick={() =>
                          onAddDevice(result.host, authInputs[result.host]?.username, authInputs[result.host]?.password)
                        }
                      >
                        Add
                      </button>
                    </div>
                    {result.authRequired && (
                      <div className='mt-2 grid gap-2 sm:grid-cols-2'>
                        <input
                          type='text'
                          className='input input-bordered input-sm w-full'
                          placeholder='Username (optional)'
                          value={authInputs[result.host]?.username || ''}
                          onChange={e => handleAuthInput(result.host, 'username', e.target.value)}
                        />
                        <input
                          type='password'
                          className='input input-bordered input-sm w-full'
                          placeholder='Password'
                          value={authInputs[result.host]?.password || ''}
                          onChange={e => handleAuthInput(result.host, 'password', e.target.value)}
                        />
                      </div>
                    )}
                  </div>
                ))}
              </div>
            )}

            {devices.length === 0 && <p className='text-sm opacity-70'>No Shelly devices added.</p>}
            {devices.map(device => (
              <div key={device.id} className='rounded border p-3 text-sm space-y-2'>
                <div className='flex flex-wrap items-center justify-between gap-2'>
                  <div>
                    <div className='font-medium'>
                      {device.name || 'Shelly'} ({device.model || 'Unknown'})
                    </div>
                    <div className='opacity-70'>{device.host}</div>
                  </div>
                  <div className='flex gap-2'>
                    <button
                      type='button'
                      className='btn btn-ghost btn-xs'
                      onClick={() => onTestDevice(device.id, 0)}
                    >
                      Test
                    </button>
                    <button
                      type='button'
                      className='btn btn-ghost btn-xs text-error'
                      onClick={() => onRemoveDevice(device.id)}
                    >
                      Remove
                    </button>
                  </div>
                </div>
                {Array.from({ length: device.channels || 1 }).map((_, idx) => (
                  <div key={`${device.id}-${idx}`} className='flex items-center justify-between gap-2'>
                    <span>Relay {idx}</span>
                    <select
                      className='select select-bordered select-sm'
                      value={String(getAssignmentValue(device.id, idx))}
                      onChange={e => handleAssign(device.id, idx, e.target.value)}
                    >
                      {functionOptions.filter(filteredOptions).map(option => (
                        <option key={option.value} value={option.value}>
                          {option.label}
                        </option>
                      ))}
                    </select>
                  </div>
                ))}
              </div>
            ))}
          </div>

        </div>
      )}
    </div>
  );
}
